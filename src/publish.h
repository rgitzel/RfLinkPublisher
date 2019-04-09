

#ifndef PUBLISH_H
#define PUBLISH_H

#include <PubSubClient.h>

#include "debug.h"

#include "wifi_lib.h"


class MqttPublisher {
    private:
        DebugSerial *_debug;

        const char *_server;
        int _port;

        WiFiClient *_wifi;
        PubSubClient *_client;

        void _connect();

    public:
        MqttPublisher(DebugSerial *debug, const char *server, const int port);

        void startup();

        bool publish(const char *topic, const char *message);
};

#endif