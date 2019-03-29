#include <Arduino.h>

#include <SoftwareSerial.h>


#define RFLINK 

SoftwareSerial debug(-1, D1);
HardwareSerial rflink = Serial;



// https://circuits4you.com/2018/12/31/esp32-hardware-debug2-example/
// #define RXD2 16
// #define TXD2 17

#define LED_PIN 2

const char* ssid = "KOBEAR-2G";
const char* password = "wookie4701";
 
const char* mqttServer = "192.168.0.13";
const int mqttPort = 1883;


void setup() {
  debug.begin(9600);

  debug.println("starting...");
  
#ifdef RFLINK
  // ESP32
  // rflink.begin(57600, SERIAL_8N12, RXD2, TXD2);
  // ESP8266
  rflink.begin(57600, SERIAL_8N1);
  rflink.swap();
#endif

  pinMode (LED_PIN, OUTPUT);
  digitalWrite (LED_PIN, LOW);
}



const int MAX_VALUES = 10;
char *nameValuePairs[MAX_VALUES];

char buffer[128];
int i = 0;

void loop() {
#ifdef RFLINK
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
    debug.print('\n');
    debug.print(buffer);
    debug.print('\n');

    // skip the first two (they're just counters)
    if(strtok(buffer, ";") && strtok(NULL, ";"))
    {
      char *device;
      char *token;
      char buffer[100];

      String json = "";

      if((device = strtok(NULL, ";")))
      {
        sprintf(buffer, "{ \"device\": \"%s\"", device);
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

        debug.println(json.c_str());
      }
    }
  }
#endif

  digitalWrite (LED_PIN, HIGH);
  delay(700);
  digitalWrite (LED_PIN, LOW);
  delay(1300);

  debug.println(i++);
}
