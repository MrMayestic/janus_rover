#ifndef SPI_COMM_H
#define SPI_COMM_H
#include "Arduino.h"
#include <SPI.h>

extern char buffer[32];
extern String dataRec;
extern uint8_t oldsrg;
extern uint8_t c;

// Forward declaration to avoid a circular include with command_handler.h
void handleIncomingRequests(String message);

void fill_buffer(String data)
{
  for (int i = 0; i <= data.length(); i++)
  {
    buffer[i] = (uint8_t)data[i];
  }
}

void sendData(String data)
{
  // Serial.println(data);
  fill_buffer(data);

  SPI.transfer(buffer, data.length() + 1);
  delayMicroseconds(5);
  SPI.transfer((uint8_t)4);
}

/* ISR that handles reciving data via SPI from Master */

ISR(SPI_STC_vect)
{
  oldsrg = SREG;

  cli();

  c = SPDR;

  if (c < 128 && c > 31)
  {
    dataRec += (char)c;
  }
  if (c == 4)
  {
    if (dataRec.length() > 0)
    {
      Serial.println(dataRec);
      handleIncomingRequests(dataRec);
    }
    dataRec = "";
  }
  SREG = oldsrg;
}

#endif
