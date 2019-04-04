
#include "serial.h"


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

