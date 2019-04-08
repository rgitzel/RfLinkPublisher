
// calling this "wifi.h" lead to problems not finding "WiFi.h"... #include
//  isn't case-sensitive?

#ifndef _WIFI_LIB_H
#define _WIFI_LIB_H


#ifdef NODEMCU
#include <ESP8266WiFi.h>
#endif

#ifdef ESP01
#include <ESP8266WiFi.h>
#endif

#ifdef ESP32
#include <WiFi.h>
#endif


#include "debug.h"


void connect_to_wifi(DebugSerial *debug, const char *ssid, const char *password);


#endif
