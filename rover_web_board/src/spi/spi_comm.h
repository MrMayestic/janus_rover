#ifndef SPI_COMM_H
#define SPI_COMM_H
#include "Arduino.h"
#include <SPI.h>

static const int spiClk = 4000000; // Clock for SPI

extern SemaphoreHandle_t spiMutex;
extern SPIClass *hspi;
extern String recivedData;
extern bool gotMessage;

/*Function that handles SPI Sending to Slave (rover main board)*/

void send_data(const String &stringMess)
{
    xSemaphoreTake(spiMutex, portMAX_DELAY);

    hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
    digitalWrite(HSPI_SS, LOW);
    delayMicroseconds(5);

    // Data transfer
    char buf[32] = {0};
    stringMess.toCharArray(buf, sizeof(buf));

    for (size_t i = 0; buf[i]; i++)
    {
        delayMicroseconds(25);
        hspi->transfer(buf[i]);
    }

    delayMicroseconds(10);
    hspi->transfer(4);

    digitalWrite(HSPI_SS, HIGH);
    hspi->endTransaction();

    xSemaphoreGive(spiMutex);
}

/*Function that reads data from Slave*/

void read_data()
{
    xSemaphoreTake(spiMutex, portMAX_DELAY);

    hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
    digitalWrite(HSPI_SS, LOW);

    recivedData = "";

    for (int i = 0; i < MAX_REC_LEN; i++)
    {
        uint8_t byteRead = hspi->transfer(0x00);

        if (byteRead == 4)
        {
            gotMessage = true;
            break;
        }

        if (byteRead >= 32 && byteRead < 128)
        {
            recivedData += char(byteRead);
        }
        delayMicroseconds(25);
    }

    digitalWrite(HSPI_SS, HIGH);
    hspi->endTransaction();

    xSemaphoreGive(spiMutex);
}

#endif