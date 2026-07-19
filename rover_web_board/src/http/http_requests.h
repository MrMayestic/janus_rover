#ifndef HTTP_REQUESTS_H
#define HTTP_REQUESTS_H
#include "Arduino.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <atomic>
#include "../spi/spi_comm.h"
#include "../sensor_data/sensor_data.h"

extern String joystickPath;
extern String sendDataPath;

extern SensorData currentSensorData;

enum class HttpRequestType
{
    Joystick,
    Telemetry
};

void httpDataRequest(HttpRequestType reqType);

#endif