
#include <Arduino.h>

#include "settings.h"


#include "publish.h"
#include "rflink.h"
#include "wifi_lib.h"




#ifdef NODEMCU
#include <SoftwareSerial.h>
SoftwareSerial debug(-1, D1);
HardwareSerial rflink = Serial;
#define LED_ON LOW
#endif

#ifdef ESP01
HardwareSerial debug = Serial1;
HardwareSerial rflink = Serial;
#define LED_ON LOW
#endif

#ifdef ESP32
// it seems you can't mix these with 'Serial' objects, they 
//  tend to override each other (e.g. using the same baud rate)
HardwareSerial debug(0);
HardwareSerial rflink(2);
#define LED_ON HIGH
#endif



#define LED_PIN 2

WiFiClient espClient;
PubSubClient mqttClient(espClient);


void setup() {
#ifdef ESP32
  debug.begin(9600, SERIAL_8N1, -1, TX);
  rflink.begin(57600, SERIAL_8N1, 16);
#else
  debug.begin(9600);
  rflink.begin(57600, SERIAL_8N1);
  rflink.swap();
#endif
  debug.println("starting...");

  connect_to_wifi(debug, ssid, password);

  pinMode (LED_PIN, OUTPUT);
  digitalWrite (LED_PIN, !LED_ON);

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
        digitalWrite (LED_PIN, LED_ON);
        delay(300);
        digitalWrite (LED_PIN, !LED_ON);
      }
    }

    if(influxTopic) {
      rflink_message_to_influx(&message, s, MAX_LENGTH_OF_OUTPUT_STRING);
      debug.printf("%s\n", s);
      if(publish(debug, mqttClient, influxTopic, s)) {
        digitalWrite (LED_PIN, LED_ON);
        delay(300);
        digitalWrite (LED_PIN, !LED_ON);
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
