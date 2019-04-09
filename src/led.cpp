
#include <Arduino.h>

#include "led.h"


Led::Led(int pin, int on_value) {
    _pin = pin;
    _on = on_value;
}

void Led::setup() {
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
