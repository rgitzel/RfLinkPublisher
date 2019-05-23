
#include <Arduino.h>

#include "pulse.h"


Pulse::Pulse(DebugSerial *debug, Led *led, int interval_millis, char ch) {
  _debug = debug;
  _led = led;
  _interval_millis = interval_millis;
  _ch = ch;
}

void Pulse::setup() {
  _last_millis = millis();
}

void Pulse::_beat() {
  if(_ch)
    _debug->print(".");
  _led->blink(50);
}

void Pulse::loop() {
  unsigned long now = millis();

  if(now < _last_millis) {
    // the value has overrun, so reset it... which will
    //  make for a slight burp every 50 days or so
    _last_millis = now;
  }
  else if(now - _last_millis > (unsigned long)_interval_millis) {
    _beat();
    _last_millis += _interval_millis;
  }
}
