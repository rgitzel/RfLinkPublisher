
#include "debug.h"
#include "rflink_message.h"
#include "rflink_reader.h"
#include "serial.h"


RfLinkReader::RfLinkReader(DebugSerial *debug) {
  _debug = debug;
}

void RfLinkReader::setup(HardwareSerial *rflink) {
  _rflink = rflink;
}

bool RfLinkReader::read(RfLinkMessage *message) {
  char buffer[MAX_LENGTH_OF_RFLINK_MESSAGE];
  
  if(read_from_serial(_rflink, buffer, MAX_LENGTH_OF_RFLINK_MESSAGE) > 0)
  {
    _debug->printf("\nreceived: '%s'\n", buffer);

    if(RfLinkMessage::from_string(buffer, message)) {
        return true;
    }
    else {
        _debug->println("ignoring unrecognized message");    
    }
  }
    // else {
    //   _debug->println("nothing there");
    //   delay(100);
    // }

  return false;
}
