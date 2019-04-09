
#ifndef _RFLINK_READER_H
#define _RFLINK_READER_H

#include <Arduino.h>

#include "debug.h"

#include "rflink_message.h"


class RfLinkReader {
  protected:
    DebugSerial *_debug;
    HardwareSerial *_rflink;


  public:
    RfLinkReader(DebugSerial *debug);
    
    void setup(HardwareSerial *rflink);

    bool read(RfLinkMessage *message);
};


#endif