#ifndef SPI_COMM_H
#define SPI_COMM_H
#include "Arduino.h"
#include <SPI.h>

// ── SPI pins (communication with MEGA 2560) ─────────────────────────
#define HSPI_MISO 12
#define HSPI_MOSI 13
#define HSPI_SCLK 14
#define HSPI_SS 15

// ── Global consts for capacity/protocols ─────────────────────────
#define MAX_REC_LEN 64 // max message length

static const int spiClk = 4000000; // Clock for SPI

extern SemaphoreHandle_t spiMutex;
extern SPIClass *hspi;
extern String recivedData;
extern bool gotMessage;

/*Function that handles SPI Sending to Slave (rover main board)*/
void send_data(const String &stringMess);

/*Function that reads data from Slave*/
void read_data();

#endif