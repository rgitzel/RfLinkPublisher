
#include <Arduino.h>

#include "settings.h"

#include "led.h"
// #include "mqtt-publisher.h"
#include "pulse.h"
// #include "wifi_lib.h"

#include <EmonLib.h>

EnergyMonitor emon;

HardwareSerial debug(0);

Led led(2, HIGH);

Pulse pulse(&debug, &led, 1000);

// MqttPublisher publisher(&debug, mqttServer, mqttPort);

const int pin1 = A0;

#define EMON 0

void setup() {
  debug.begin(9600, SERIAL_8N1, -1, TX);
  
  debug.println("starting...");

  led.setup();
  pulse.setup();

  // connect_to_wifi(&debug, ssid, password);
  // publisher.setup();

#if EMON
  emon.current(A0, 30);
  analogSetAttenuation(ADC_6db);
#else
  // emon sets this to 10?
  analogReadResolution(12);

  analogSetAttenuation(ADC_11db);

  adcAttachPin(pin1);
  adcStart(pin1);
#endif
}



void poll_ct() {
#if EMON
  debug.printf("%f\n", emon.calcIrms(1480));
#else
  double acc = 0;
  double acc2 = 0;
  
  const int N = 40000;

  for(int i = 0 ; i < N; i++) {
    int reading = analogRead(pin1) - 1675.0;
    acc += reading;
    acc2 += (reading * reading);
  }
  debug.printf("%.1f %.1f %.1f %.1f\n", 
    acc / N, 
    sqrt(acc2/N), 
    sqrt(acc2/N) * 9.7 / 270.0, 
    sqrt(acc2/N) * 12.6 / 380.0);
#endif
}




void loop() {
  poll_ct();

  // delay(177);

  // pulse.loop();
}
