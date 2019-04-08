
#include <Arduino.h>

#include "led.h"


Led::Led(int pin) {
    _pin = pin;

#ifdef NODEMCU
    _on = LOW;
#endif

#ifdef ESP01
    _on = HIGH;
#endif

#ifdef ESP32
    _on = HIGH;
#endif
}

void Led::startup() {
    pinMode(_pin, OUTPUT);
    off();
}

void Led::on() {
    digitalWrite(_pin, _on);
}

void Led::off() {
    digitalWrite(_pin, !_on);
}

void Led::blink(int millis) {
    on();
    delay(millis);
    off();
}