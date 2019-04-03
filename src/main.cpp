
#define RFLINK 

#ifdef NODEMCU
#include <Arduino.h>
#include <SoftwareSerial.h>
SoftwareSerial debug(-1, D1);
#include <ESP8266WiFi.h>
#endif

#ifdef ESP01
#include <Arduino.h>
HardwareSerial debug = Serial1;
#endif

#ifdef ESP32
// https://circuits4you.com/2018/12/31/esp32-hardware-debug2-example/
#define RXD2 16
#define TXD2 17
HardwareSerial debug = Serial2;
#include <WiFi.h>
#endif


#include <PubSubClient.h>

HardwareSerial rflink = Serial;



#define LED_PIN 2

const char* ssid = "KOBEAR-2G";
const char* password = "wookie4701";
 
const char* mqttServer = "192.168.0.13";
const int mqttPort = 1883;

WiFiClient espClient;
PubSubClient mqttClient(espClient);


void setup() {
  debug.begin(9600);

  debug.println("starting...");
  
#ifdef RFLINK
  #ifdef ESP32
    rflink.begin(57600, SERIAL_8N1, RXD2, TXD2);
  #else
    rflink.begin(57600, SERIAL_8N1);
    rflink.swap();
  #endif
#endif

  pinMode (LED_PIN, OUTPUT);
  digitalWrite (LED_PIN, LOW);

#ifdef ESP32
  WiFi.setSleep(false);
#endif

  int n = WiFi.scanNetworks();
  debug.print(n);
  debug.println(" networks found");
  for(int i = 0; i < n; i++)
  {
    debug.println(WiFi.SSID(i));
  }

  debug.printf("Establishing connection to %s", ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    debug.print(".");
    delay(500);
  }
  debug.println();

  debug.printf("Connected as %s\n", WiFi.localIP().toString().c_str());
  debug.println();


  mqttClient.setServer(mqttServer, mqttPort);
}



const int MAX_VALUES_IN_RFLINK_MESSAGE = 10;
const int MAX_LENGTH_OF_RFLINK_MESSAGE = 64;
const int MAX_LENGTH_OF_JSON_DOCUMENT = 128;

int read_from_serial(HardwareSerial hs, char *buffer, int max_length) {
  char next;
  int numCharsRead = 0;

  while(
    (hs.available() > 0)
      && ((next = char(hs.read())) != '\n')
      && (numCharsRead < max_length)
  )    
  {
    buffer[numCharsRead++] = next;
    delay(1);
  }
  buffer[numCharsRead] = '\0';

  return numCharsRead;
}

bool send_to_mqtt(PubSubClient mqttClient, const char *topic, const char *message) {
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


typedef struct {
  char name[16];
  char value[16];
} NameValuePair;

bool has_hex_value(char *name) {
  return !strcmp(name, "TEMP") ||
         !strcmp(name, "BARO") ||
         !strcmp(name, "UV") ||
         !strcmp(name, "LUX") ||
         !strcmp(name, "RAIN") ||
         !strcmp(name, "RAINRATE") ||
         !strcmp(name, "WINSP") ||
         !strcmp(name, "AWINSP") ||
         !strcmp(name, "WINGS") ||
         !strcmp(name, "WINCHL") ||
         !strcmp(name, "WINTMP") ||
         !strcmp(name, "KWATT") ||
         !strcmp(name, "WATT");
}

bool should_divide_by_10(char *name) {
  return !strcmp(name, "TEMP") ||
         !strcmp(name, "RAINRATE") ||
         !strcmp(name, "WINSP") ||
         !strcmp(name, "AWINSP");
}

bool has_integer_value(char *name) {
  return !strcmp(name, "BFORECAST") ||
         !strcmp(name, "CHIME") ||
         !strcmp(name, "CO2") ||
         !strcmp(name, "CURRENT") ||
         !strcmp(name, "CURRENT2") ||
         !strcmp(name, "CURRENT3") ||
         !strcmp(name, "DIST") ||
         !strcmp(name, "HSTATUS") ||
         !strcmp(name, "HUM") ||
         !strcmp(name, "METER") ||
         !strcmp(name, "SET_LEVEL") ||
         !strcmp(name, "SOUND") ||
         !strcmp(name, "VOLT") ||
         !strcmp(name, "WINDDIR");
}

void to_pairs(char *pairStrings[], NameValuePair *pairs, int numPairs) {
  for(int i = 0 ; i < numPairs; i++)
  {
    char *name = strtok(pairStrings[i], "=");
    char *value = strtok(NULL, "=");

    size_t j;
    for(j = 0; j < strlen(name) && j < 15; j++) {
      pairs[i].name[j] = tolower(name[j]);
    }
    pairs[i].name[j] = '\0';
    
    if(has_hex_value(name)) {
      int n;
      sscanf(value, "%x", &n);
      if(should_divide_by_10(name)) {
        sprintf(pairs[i].value, "%.1f", n * 0.1);
      }
      else {
        sprintf(pairs[i].value, "%d", n);
      }
    }
    else if(has_integer_value(name)) {
      int n;
      sscanf(value, "%d", &n);
      sprintf(pairs[i].value, "%d", n);
    }
    else {
      sprintf(pairs[i].value, "\"%s\"", value);
    }
  }
}

void pairs_to_json(char *device, char *id, NameValuePair *pairs, int numPairs, char *jsonString, int max_length) {
  String json = "{\"device\":\"";
  json += device;
  json += "\",\"device_id\":\"";
  json += id;
  json += "\",\"data\":{";

  for(int i = 0 ; i < numPairs; i++)
  {
    if(i > 0)
      json += ",";
    json += "\"";
    json += pairs[i].name;
    json += "\":";
    json += pairs[i].value;
  }

  json += "}}";

  strncpy(jsonString, json.c_str(), max_length);
}

void pairs_to_influx(char *device, char *id, NameValuePair *pairs, int numPairs, char *str, int max_length) {
  // measurement
  String influx = "rflink";

  // tags
  influx += ",device=";
  influx += device;
  influx += ",device_id=";
  influx += id;
  influx += " ";

  // values
  for(int i = 0 ; i < numPairs; i++)
  {
    if(i > 0)
      influx += ",";
    influx += pairs[i].name;
    influx += "=";
    influx += pairs[i].value;
  }

  strncpy(str, influx.c_str(), max_length);
}


void publish(PubSubClient mqtt_server, const char *topic, const char *message) {
  if(send_to_mqtt(mqttClient, topic, message))
  {
    debug.printf("successfully published to '%s'\n", topic);
    digitalWrite (LED_PIN, LOW);
    delay(300);
    digitalWrite (LED_PIN, HIGH);
  }
  else
  {
    debug.printf("failed to publish to '%s'\n", topic);
  }        
}

void poll_rflink() {
  char buffer[MAX_LENGTH_OF_RFLINK_MESSAGE];

  if(read_from_serial(rflink, buffer, MAX_LENGTH_OF_RFLINK_MESSAGE) > 0)
  {
    debug.print('\n');
    debug.print(buffer);
    debug.print('\n');

    // skip the first two (they're just counters)
    if(strtok(buffer, ";") && strtok(NULL, ";"))
    {
      char *device, *id;

      if((device = strtok(NULL, ";")) && (id = strtok(NULL, ";")))
      {
        // skip over "ID="
        id = id + 3;

        char *token;
        char *nameValuePairStrings[MAX_VALUES_IN_RFLINK_MESSAGE];
        NameValuePair nameValuePairs[MAX_VALUES_IN_RFLINK_MESSAGE];

        int numPairs = 0;
        while((token = strtok(NULL, ";")) && (numPairs < MAX_VALUES_IN_RFLINK_MESSAGE))
        {
          while(isspace(*token)) token++;
          if(strlen(token) > 0)
            nameValuePairStrings[numPairs++] = token;
        }

        to_pairs(nameValuePairStrings, nameValuePairs, numPairs);

        char s[MAX_LENGTH_OF_JSON_DOCUMENT];

        pairs_to_json(device, id, nameValuePairs, numPairs, s, MAX_LENGTH_OF_JSON_DOCUMENT);
        debug.printf("%s\n", s);
        publish(mqttClient, "foo", s);

        pairs_to_influx(device, id, nameValuePairs, numPairs, s, MAX_LENGTH_OF_JSON_DOCUMENT);
        debug.printf("%s\n", s);
        publish(mqttClient, "influx/input", s);

      } 
    }
  }
}



int odd = 0;
int i = 0;

void loop() {
#ifdef RFLINK
  poll_rflink();
#endif

  delay(10);

  if(++i % 100 == 0) {
  debug.print(odd ? "-" : "^" );
  odd = !odd;
    i = 0; 
  }
}
