
#include "serial.h"


int read_from_serial(HardwareSerial *hs, char *buffer, int max_length) {
  char next;
  int numCharsRead = 0;

  while(
    (hs->available() > 0)
      && (numCharsRead < max_length)
      && ((next = char(hs->read())))
      && (next != '\n')
  )    
  {
    buffer[numCharsRead++] = next;
    delay(1);
  }
  if(numCharsRead > 0)
    buffer[numCharsRead-1] = '\0';
  else
    buffer[0] = '\0';

  return numCharsRead;
}

