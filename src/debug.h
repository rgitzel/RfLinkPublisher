
#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

/*
 * normally you would just use a UART TX line to emit console logs, but
 *  the NodeMCU board inconveniently uses that line for its LED.  So for
 *  that board, we'll use the "software" serial line.
 * 
 * Which is all fine, until you want to pass along the Serial object,
 *  and you find that HardwareSerial and SoftwareSerial do not share a 
 *  common parent, so ... we use this trickery to decide at compile time
 *  which one to use.
 */ 

#ifdef NODEMCU
#include <SoftwareSerial.h>
typedef SoftwareSerial DebugSerial;
#endif

#ifdef ESP01
#include <SoftwareSerial.h>
typedef SoftwareSerial DebugSerial;
#endif


#ifdef ESP32
typedef HardwareSerial DebugSerial;
#endif

#endif