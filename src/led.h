
#ifndef _LED_H
#define _LED_H


class Led {
    private:
        int _pin;
        int _on;

    public:
        Led(int pin, int on_value);
        
        void setup();
        
        void on();
        void off();

        void blink(int millis);
};


#endif
