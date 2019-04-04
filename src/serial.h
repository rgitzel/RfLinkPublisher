
#ifndef SERIAL_H
#define SERIAL_H

#include <Arduino.h>


int read_from_serial(HardwareSerial hs, char *buffer, int max_length);

#endif