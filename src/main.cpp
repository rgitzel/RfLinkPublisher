
#include <Arduino.h>

#include "settings.h"

#include "led.h"
#include "mqtt-publisher.h"
#include "pulse.h"
#include "rflink_reader.h"
#include "wifi_lib.h"


#ifdef NODEMCU
#include <SoftwareSerial.h>
SoftwareSerial debug(-1, D1);
HardwareSerial rflink = Serial;
// internal blue LED
Led led(2, LOW);
#endif

#ifdef ESP01
#include <SoftwareSerial.h>
// debug will be on GPIO0, not on TX!
SoftwareSerial debug(-1, 0);
HardwareSerial rflink(0);
// internal blue LED on GPIO1 (shared with TX)
Led led(1, LOW);
#endif

#ifdef ESP32
// it seems you can't mix these with 'Serial' objects, they 
//  tend to override each other (e.g. using the same baud rate)
HardwareSerial debug(0);
HardwareSerial rflink(2);
Led led(2, HIGH);
#endif




Pulse pulse(&debug, &led, 1000, 'x');

RfLinkReader reader(&debug);
MqttPublisher publisher(&debug, mqttServer, mqttPort);

unsigned long last_millis;


void setup() {
#ifdef ESP32
  debug.begin(9600, SERIAL_8N1, -1, TX);
  rflink.begin(57600, SERIAL_8N1, 16);
#else
  debug.begin(9600);
#ifdef NODEMCU
  rflink.begin(57600, SERIAL_8N1, SERIAL_RX_ONLY);
  rflink.swap();
#else
  rflink.begin(57600, SERIAL_8N1, SERIAL_RX_ONLY);
  // RX seems to default to being a GPIO pin :(
  pinMode(3, FUNCTION_0);
#endif
#endif
  
  debug.println("starting...");

  led.setup();

  connect_to_wifi(&debug, ssid, password);

  publisher.setup();

  reader.setup(&rflink);

  pulse.setup();
}



const int MAX_LENGTH_OF_OUTPUT_STRING = 256;

void poll_rflink() {
  RfLinkMessage message;

  if(reader.read(&message) > 0)
  {
    debug.printf("got message\n");

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




void loop() {
  poll_rflink();

  // don't need to poll like crazy in a super tight loop... even this is probably overkill
  delay(100);

  pulse.loop();
}
