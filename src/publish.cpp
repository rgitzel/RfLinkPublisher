
#include <publish.h>


bool send_to_mqtt(DebugSerial debug, PubSubClient mqttClient, const char *topic, const char *message) {
  while (!mqttClient.connected()) {
    debug.println("ESP > Connecting to MQTT...");

    if (mqttClient.connect("foo")) {
      debug.println("connected to MQTT server");
    } else {
      debug.print("ERROR > failed with state ");
      debug.print(mqttClient.state());
      delay(1000);

    }
  }

  return mqttClient.publish(topic, message) == 1;
}

bool publish(DebugSerial debug, PubSubClient mqttClient, const char *topic, const char *message) {
  if(send_to_mqtt(debug, mqttClient, topic, message))
  {
    debug.printf("successfully published to '%s'\n", topic);
    return true;
  }
  
  debug.printf("failed to publish to '%s'\n", topic);
  
  return false;
}

