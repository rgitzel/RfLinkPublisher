
#include "mqtt-publisher.h"


MqttPublisher::MqttPublisher(DebugSerial *debug, const char *server, int port) {
  _debug = debug;
  _server = server;
  _port = port;
}

void MqttPublisher::setup() {
  // we have to explicitly keep this around, or PubSubClient will fail mysteriously
  _wifi = new WiFiClient;
  _client = new PubSubClient(*_wifi);
  _client->setServer(_server, _port);
  _connect();
}

void MqttPublisher::_connect() {
  while (!_client->connected()) {
    _debug->println("Connecting to MQTT...");
    if (_client->connect("foo")) {
      _debug->printf("connected to MQTT server '%s'\n", _server);
    } else {
      _debug->printf("ERROR: connecting to MQTT failed with '%d'\n", _client->state());
      delay(1000);
    }
  }
}

bool MqttPublisher::publish(const char *topic, const char *message) {
  _connect();
  if(_client->publish(topic, message)) {
    _debug->printf("successfully published to '%s'\n", topic);
    return true;
  }
  _debug-> printf("failed to publish to '%s'\n", topic);
  return false;
}
