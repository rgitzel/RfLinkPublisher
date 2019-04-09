
#include "serial.h"


int read_from_serial(HardwareSerial *hs, char *buffer, int max_length) {
  char next;
  int numCharsRead = 0;

  while(
    (hs->available() > 0)
      && (numCharsRead < max_length)
      // && ((next = char(hs->read())) != '\n')
      && ((next = char(hs->read())))
  )    
  {
    buffer[numCharsRead++] = next;
    delay(1);
  }
  buffer[numCharsRead] = '\0';

  return numCharsRead;
}

