#include <Arduino.h>

#include <WiFi.h>
 
#include <PubSubClient.h>



HardwareSerial monitor = Serial;
HardwareSerial rflink = Serial2;


// https://circuits4you.com/2018/12/31/esp32-hardware-monitor2-example/
#define RXD2 16
#define TXD2 17

#define LED_PIN 2

const char* ssid = "KOBEAR-2G";
const char* password = "wookie4701";
 
const char* mqttServer = "192.168.0.13";
const int mqttPort = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  monitor.begin(9600);

  monitor.printf("Establishing connection to %s", ssid);

  // took a long time to find that this is critical for the ESP32

  WiFi.setSleep(false);

  int n = WiFi.scanNetworks();
  monitor.print(n);
  monitor.println(" networks found");
  for(int i = 0; i < n; i++)
  {
    monitor.println(WiFi.SSID(i));
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    monitor.print(".");
    delay(500);
  }
  monitor.println();

  monitor.print("Connected as ");
  monitor.print(WiFi.localIP());
  monitor.println();


  client.setServer(mqttServer, mqttPort);
  while (!client.connected()) {
    monitor.println("ESP > Connecting to MQTT...");
 
    if (client.connect("foo")) {
      monitor.println("connected to MQTT server");
    } else {
      monitor.print("ERROR > failed with state ");
      monitor.print(client.state());
      delay(2000);
 
    }
  }


  rflink.begin(57600, SERIAL_8N1, RXD2, TXD2);

  pinMode (LED_PIN, OUTPUT);
  digitalWrite (LED_PIN, LOW);
}



const int MAX_VALUES = 10;
char *nameValuePairs[MAX_VALUES];

char buffer[128];

void loop() {

  char next;
  int numCharsRead = 0;
  while((rflink.available() > 0) && ((next = char(rflink.read())) != '\n') && (numCharsRead < 128))
  {
    buffer[numCharsRead++] = next;
    delay(1);
  }
  buffer[numCharsRead] = '\0';

  if(numCharsRead > 0)
  {
    monitor.print('\n');
    monitor.print(buffer);
    monitor.print('\n');

    // skip the first two (they're just counters)
    if(strtok(buffer, ";") && strtok(NULL, ";"))
    {
      char *token;
      char buffer[100];

      String json = "";

      if((token = strtok(NULL, ";")))
      {
        sprintf(buffer, "{ \"device\": \"%s\"", token);
        json += buffer;

        int numPairs = 0;
  
        while((token = strtok(NULL, ";")) && (numPairs < MAX_VALUES))
        {
          nameValuePairs[numPairs++] = token;
        }

        for(int i = 0 ; i < numPairs; i++)
        {
          char *name = strtok(nameValuePairs[i], "=");
          char *value = strtok(NULL, "=");
          if(name && value)
          {
            sprintf(buffer, ", \"%s\": \"%s\"", name, value);
            json += buffer;
          }
        }

        json += " }";

        monitor.println(json.c_str());
        monitor.println(client.publish("test/rflink", json.c_str()));
      }
    }
    
    digitalWrite (LED_PIN, HIGH);
    delay(300);
    digitalWrite (LED_PIN, LOW);
  }
  else {
    delay(300);
  }

  delay(700);
  monitor.print(".");
}
