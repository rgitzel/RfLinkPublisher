

#ifndef PUBLISH_H
#define PUBLISH_H

#include <PubSubClient.h>

#include "debug.h"


bool publish(DebugSerial debug, PubSubClient mqtt_server, const char *topic, const char *message);

#endif