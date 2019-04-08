
#include <Arduino.h>

#include "settings.h"

#include "led.h"
#include "publish.h"
#include "rflink_reader.h"
#include "wifi_lib.h"




#ifdef NODEMCU
#include <SoftwareSerial.h>
SoftwareSerial debug(-1, D1);
HardwareSerial rflink = Serial;
// internal blue LED
#define LED_PIN 2
#endif

#ifdef ESP01
#include <SoftwareSerial.h>
SoftwareSerial debug(-1, 0);
HardwareSerial rflink = Serial;
// external LED (internal blue one is shared with TX)
#define LED_PIN 2
#endif

#ifdef ESP32
// it seems you can't mix these with 'Serial' objects, they 
//  tend to override each other (e.g. using the same baud rate)
HardwareSerial debug(0);
HardwareSerial rflink(2);
#define LED_PIN 2
#endif





Led led(LED_PIN);
RfLinkReader reader(&debug);
MqttPublisher publisher(&debug, mqttServer, mqttPort);

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

  led.startup();

  connect_to_wifi(&debug, ssid, password);

  publisher.startup();

  reader.startup(&rflink);
}



const int MAX_LENGTH_OF_OUTPUT_STRING = 128;

void poll_rflink() {
  RfLinkMessage message;

  if(reader.read(&message) > 0)
  {
    char s[MAX_LENGTH_OF_OUTPUT_STRING];

    if(jsonTopic) {
      message.to_json(s, MAX_LENGTH_OF_OUTPUT_STRING);
      debug.printf("%s\n", s);
      if(publisher.publish(jsonTopic, s)) {
        led.blink(300);
      }
    }

    if(influxTopic) {
      message.to_influx(s, MAX_LENGTH_OF_OUTPUT_STRING);
      debug.printf("%s\n", s);
      if(publisher.publish(influxTopic, s)) {
        led.blink(300);
      }
    }
  }
}




unsigned long last_millis = millis();

void heartbeat() {
  debug.print(".");
  led.blink(100);
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
