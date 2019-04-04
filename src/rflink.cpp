
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

bool read_from_rflink(DebugSerial debug, HardwareSerial rflink, RflinkMessage *message) {
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
        strcpy(message->device, device);
        
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

        return true;
      } 
    }
  }
  return false;
}
