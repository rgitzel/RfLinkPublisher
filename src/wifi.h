
#ifndef WIFI_H
#define WIFI_H


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


void connect_to_wifi(DebugSerial debug, const char *ssid, const char *password);


#endif
