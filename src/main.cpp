
#include <Arduino.h>

#include "settings.h"


#ifdef NODEMCU
#include <SoftwareSerial.h>
SoftwareSerial debug(-1, D1);
#endif

#ifdef ESP01
HardwareSerial debug = Serial1;
#endif

#ifdef ESP32
// https://circuits4you.com/2018/12/31/esp32-hardware-debug2-example/
#define RXD2 16
#define TXD2 17
HardwareSerial debug = Serial2;
#endif


#include "publish.h"
#include "rflink.h"
#include "wifi.h"



HardwareSerial rflink = Serial;



#define LED_PIN 2

WiFiClient espClient;
PubSubClient mqttClient(espClient);


void setup() {
  debug.begin(9600);
  debug.println("starting...");

  connect_to_wifi(debug, ssid, password);

#ifdef ESP32
  rflink.begin(57600, SERIAL_8N1, RXD2);
#else
  rflink.begin(57600, SERIAL_8N1);
  rflink.swap();
#endif

  pinMode (LED_PIN, OUTPUT);
  digitalWrite (LED_PIN, LOW);

  mqttClient.setServer(mqttServer, mqttPort);
}



const int MAX_LENGTH_OF_OUTPUT_STRING = 128;

void poll_rflink() {
  RflinkMessage message;

  if(read_from_rflink(debug, rflink, &message) > 0)
  {
    char s[MAX_LENGTH_OF_OUTPUT_STRING];

    if(jsonTopic) {
      rflink_message_to_json(&message, s, MAX_LENGTH_OF_OUTPUT_STRING);
      debug.printf("%s\n", s);
      if(publish(debug, mqttClient, jsonTopic, s)) {
        digitalWrite (LED_PIN, LOW);
        delay(300);
        digitalWrite (LED_PIN, HIGH);
      }
    }

    if(influxTopic) {
      rflink_message_to_influx(&message, s, MAX_LENGTH_OF_OUTPUT_STRING);
      debug.printf("%s\n", s);
      if(publish(debug, mqttClient, influxTopic, s)) {
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

  // don't need to poll like crazy in a super tight loop... even this is probably overkill
  delay(10);

  unsigned long now = millis();
  if(now < last_millis) {
    // the value has overrun, so reset it... which will
    //  make for a slight burp every 50 days or so
    last_millis = now;
  }
  else if(now - last_millis > 1000) {
    heartbeat();
    last_millis += 1000;
  }
}
