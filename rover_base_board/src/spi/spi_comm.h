#ifndef SPI_COMM_H
#define SPI_COMM_H
#include "Arduino.h"
#include <SPI.h>

extern String dataRec;
extern uint8_t oldsrg;
extern uint8_t c;
extern volatile bool commandReady;

volatile uint8_t txBuffer[33];
volatile uint8_t txLength = 0;
volatile uint8_t txIndex = 0;

// Forward declaration to avoid a circular include with command_handler.h
void handleIncomingRequests(String message);

void fill_buffer(const String &data)
{
  size_t len = min(data.length(), sizeof(txBuffer) - 1);

  for (size_t i = 0; i < len; i++)
  {
    txBuffer[i] = (uint8_t)data[i];
  }

  txBuffer[len] = 4;
  txLength = len + 1;
}

void sendData(const String &data)
{
  noInterrupts();
  fill_buffer(data);
  txIndex = 1;
  SPDR = txBuffer[0];
  interrupts();
}

/* ISR that handles data transfer via SPI */

ISR(SPI_STC_vect)
{
  oldsrg = SREG;

  cli();

  c = SPDR;

  if (txIndex < txLength)
    SPDR = txBuffer[txIndex++];
  else
    SPDR = 0;

  if (c < 128 && c > 31)
  {
    dataRec += (char)c;
  }
  if (c == 4)
  {
    if (!commandReady && dataRec.length() > 0)
    {
      commandReady = true;
    }
    else
    {
      dataRec = "";
    }
  }

  SREG = oldsrg;
}

#endif
