
#include "rflink.h"
#include "serial.h"


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

bool parse_message(DebugSerial debug, char *buffer, RflinkMessage *message) {
    bool recognized = false;

    // skip the first two (they're just counters)
    if(strtok(buffer, ";") && strtok(NULL, ";"))
    {
        char *device, *id;

        if((device = strtok(NULL, ";")) && (id = strtok(NULL, ";")))
        {
            strcpy(message->device, device);
                
            // for now assume that first pair will be the ID, and if not
            //  it's not a message we can process
            if(!strncmp("ID=", id, 3)) {
                // skip over "ID="
                strcpy(message->id, id + 3);

                char *token;
                char *nameValuePairStrings[MAX_VALUES_IN_RFLINK_MESSAGE];

                int numPairs = 0;
                while((token = strtok(NULL, ";")) && (numPairs < MAX_VALUES_IN_RFLINK_MESSAGE))
                {
                    while(isspace(*token)) token++;
                    if(strlen(token) > 0)
                    nameValuePairStrings[numPairs++] = token;
                }
                
                to_pairs(nameValuePairStrings, message->values, numPairs);
                message->numValues = numPairs;

                recognized = true;
            }
        } 
    }

    return recognized;
}

bool read_from_rflink(DebugSerial debug, HardwareSerial rflink, RflinkMessage *message) {
  char buffer[MAX_LENGTH_OF_RFLINK_MESSAGE];
  
  if(read_from_serial(rflink, buffer, MAX_LENGTH_OF_RFLINK_MESSAGE) > 0)
  {
    debug.printf("\nreceived: '%s'\n", buffer);

    if(parse_message(debug, buffer, message)) {
        return true;
    }
    else {
        debug.println("ignoring unrecognized message");    
    }
  }

  return false;
}



void rflink_message_to_json(RflinkMessage *message, char *jsonString, int max_length) {
  String json = "{\"device\":\"";
  json += message->device;
  json += "\",\"device_id\":\"";
  json += message->id;
  json += "\",\"data\":{";

  for(int i = 0 ; i < message->numValues; i++)
  {
    if(i > 0)
      json += ",";
    json += "\"";
    json += message->values[i].name;
    json += "\":";
    json += message->values[i].value;
  }

  json += "}}";

  strncpy(jsonString, json.c_str(), max_length);
}

void rflink_message_to_influx(RflinkMessage *message, char *str, int max_length) {
  // measurement
  String influx = "rflink";

  // tags
  influx += ",device=";
  influx += message->device;
  influx += ",device_id=";
  influx += message->id;
  influx += " ";

  // values
  for(int i = 0 ; i < message->numValues; i++)
  {
    if(i > 0)
      influx += ",";
    influx += message->values[i].name;
    influx += "=";
    influx += message->values[i].value;
  }

  strncpy(str, influx.c_str(), max_length);
}

