
#define RFLINK 

#include <Arduino.h>

#ifdef NODEMCU
#include <SoftwareSerial.h>
SoftwareSerial debug(-1, D1);
#include <ESP8266WiFi.h>
#endif

#ifdef ESP01
HardwareSerial debug = Serial1;
#include <ESP8266WiFi.h>
#endif

#ifdef ESP32
// https://circuits4you.com/2018/12/31/esp32-hardware-debug2-example/
#define RXD2 16
#define TXD2 17
HardwareSerial debug = Serial2;
#include <WiFi.h>
#endif


#include "publish.h"
#include "rflink.h"

HardwareSerial rflink = Serial;



#define LED_PIN 2

const char* ssid = "KOBEAR-2G";
const char* password = "wookie4701";
 
const char* mqttServer = "192.168.0.13";
const int mqttPort = 1883;

WiFiClient espClient;
PubSubClient mqttClient(espClient);


void setup() {
  debug.begin(9600);

  debug.println("starting...");
  
#ifdef RFLINK
  #ifdef ESP32
    rflink.begin(57600, SERIAL_8N1, RXD2, TXD2);
  #else
    rflink.begin(57600, SERIAL_8N1);
    rflink.swap();
  #endif
#endif

  pinMode (LED_PIN, OUTPUT);
  digitalWrite (LED_PIN, LOW);

#ifdef ESP32
  WiFi.setSleep(false);
#endif

  int n = WiFi.scanNetworks();
  debug.print(n);
  debug.println(" networks found");
  for(int i = 0; i < n; i++)
  {
    debug.println(WiFi.SSID(i));
  }

  debug.printf("Establishing connection to %s", ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    debug.print(".");
    delay(500);
  }
  debug.println();

  debug.printf("Connected as %s\n", WiFi.localIP().toString().c_str());
  debug.println();


  mqttClient.setServer(mqttServer, mqttPort);
}



const int MAX_LENGTH_OF_OUTPUT_STRING = 128;

const char* JSON_TOPIC = "foo";
const char* INFLUX_TOPIC = "influx/input";

void poll_rflink() {
  RflinkMessage message;

  if(read_from_rflink(debug, rflink, &message) > 0)
  {
    char s[MAX_LENGTH_OF_OUTPUT_STRING];

    if(JSON_TOPIC) {
      rflink_message_to_json(&message, s, MAX_LENGTH_OF_OUTPUT_STRING);
      debug.printf("%s\n", s);
      if(publish(debug, mqttClient, JSON_TOPIC, s)) {
        digitalWrite (LED_PIN, LOW);
        delay(300);
        digitalWrite (LED_PIN, HIGH);
      }
    }

    if(INFLUX_TOPIC) {
      rflink_message_to_influx(&message, s, MAX_LENGTH_OF_OUTPUT_STRING);
      debug.printf("%s\n", s);
      if(publish(debug, mqttClient, INFLUX_TOPIC, s)) {
        digitalWrite (LED_PIN, LOW);
        delay(300);
        digitalWrite (LED_PIN, HIGH);
      }
    }
  }
}




unsigned long last_millis = millis();

void heartbeat() {
  debug.print(".");
}

void loop() {
  poll_rflink();

  delay(10);

  unsigned long now = millis();
  if(now < last_millis) {
    // the value has overrun, reset it... which will make for a slight burp every 50 days
    last_millis = now;
  }
  else if(now - last_millis > 1000) {
    heartbeat();
    last_millis = now;
  }
}
