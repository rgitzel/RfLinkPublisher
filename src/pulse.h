
#ifndef _PULSE_H
#define _PULSE_H

#include "debug.h"
#include "led.h"


class Pulse {
    private:
        int _interval_millis;
        unsigned long _last_millis;
        DebugSerial *_debug;
        Led *_led;

        void _beat();

    public:
        Pulse(DebugSerial *debug, Led *led, int interval_millis);

        void setup();
        void loop();
};

#endif