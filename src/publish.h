

#ifndef PUBLISH_H
#define PUBLISH_H

#include <PubSubClient.h>

#include "debug.h"


bool send_to_mqtt(DebugSerial debug, PubSubClient mqttClient, const char *topic, const char *message);

bool publish(DebugSerial debug, PubSubClient mqtt_server, const char *topic, const char *message);

#endif