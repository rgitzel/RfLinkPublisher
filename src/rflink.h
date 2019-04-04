
#ifndef RFLINK_H
#define RFLINK_H

#include <Arduino.h>

#include "debug.h"

const int MAX_VALUES_IN_RFLINK_MESSAGE = 10;
const int MAX_LENGTH_OF_RFLINK_MESSAGE = 64;


typedef struct {
  char name[16];
  char value[16];
} NameValuePair;

typedef struct {    
    char device[16];
    char id[16];
    NameValuePair values[MAX_VALUES_IN_RFLINK_MESSAGE];
    int numValues;
} RflinkMessage;

bool read_from_rflink(DebugSerial debug, HardwareSerial rflink, RflinkMessage *message);

void rflink_message_to_influx(RflinkMessage *message, char *str, int max_length);
void rflink_message_to_json(RflinkMessage *message, char *jsonString, int max_length);

#endif