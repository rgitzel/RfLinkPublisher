
#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

#ifdef NODEMCU
#include <SoftwareSerial.h>
typedef SoftwareSerial DebugSerial;
#else
typedef HardwareSerial DebugSerial;
#endif

#endif