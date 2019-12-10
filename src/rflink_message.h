
#ifndef _RFLINK_MESSAGE_H
#define _RFLINK_MESSAGE_H

#include "debug.h"

const int MAX_VALUES_IN_RFLINK_MESSAGE = 10;
const int MAX_LENGTH_OF_RFLINK_MESSAGE = 64;


typedef struct {
  char name[16];
  char value[16];
} NameValuePair;

class RfLinkMessage {
  protected:
    char _original[MAX_LENGTH_OF_RFLINK_MESSAGE];
    char _device[32];
    char _id[16];
    NameValuePair _values[MAX_VALUES_IN_RFLINK_MESSAGE];
    int _numValues;

    static bool has_integer_value(char *name);
    static bool should_divide_by_10(char *name);
    static bool has_hex_value(char *name);
    static bool parse_message(char *buffer, RfLinkMessage *message);
    static void to_pairs(char *pairStrings[], NameValuePair *pairs, int numPairs);

 public:
    RfLinkMessage();
    void to_influx(char *str, int max_length);
    void to_json(char *str, int max_length);

    static bool from_string(char *rflink_message, RfLinkMessage *message);
};

#endif