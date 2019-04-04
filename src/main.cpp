
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



const int MAX_LENGTH_OF_JSON_DOCUMENT = 128;




void pairs_to_json(RflinkMessage *message, char *jsonString, int max_length) {
  String json = "{\"device\":\"";
  json += message->device;
  json += "\",\"device_id\":\"";
  json += message->id;
  json += "\",\"data\":{";

  for(int i = 0 ; i < message->numValues; i++)
  {
    if(i > 0)
      json += ",";
    json += "\"";
    json += message->values[i].name;
    json += "\":";
    json += message->values[i].value;
  }

  json += "}}";

  strncpy(jsonString, json.c_str(), max_length);
}

void pairs_to_influx(RflinkMessage *message, char *str, int max_length) {
  // measurement
  String influx = "rflink";

  // tags
  influx += ",device=";
  influx += message->device;
  influx += ",device_id=";
  influx += message->id;
  influx += " ";

  // values
  for(int i = 0 ; i < message->numValues; i++)
  {
    if(i > 0)
      influx += ",";
    influx += message->values[i].name;
    influx += "=";
    influx += message->values[i].value;
  }

  strncpy(str, influx.c_str(), max_length);
}


void poll_rflink() {
  RflinkMessage message;

  if(read_from_rflink(debug, rflink, &message) > 0)
  {
    char s[MAX_LENGTH_OF_JSON_DOCUMENT];

    pairs_to_json(&message, s, MAX_LENGTH_OF_JSON_DOCUMENT);
    debug.printf("%s\n", s);
    if(publish(debug, mqttClient, "foo", s)) {
      digitalWrite (LED_PIN, LOW);
      delay(300);
      digitalWrite (LED_PIN, HIGH);
    }

    pairs_to_influx(&message, s, MAX_LENGTH_OF_JSON_DOCUMENT);
    debug.printf("%s\n", s);
    if(publish(debug, mqttClient, "influx/input", s)) {
      digitalWrite (LED_PIN, LOW);
      delay(300);
      digitalWrite (LED_PIN, HIGH);
    }
  }
}



int odd = 0;
int i = 0;

void loop() {
#ifdef RFLINK
  poll_rflink();
#endif

  delay(10);

  if(++i % 100 == 0) {
  debug.print(odd ? "-" : "^" );
  odd = !odd;
    i = 0; 
  }
}
