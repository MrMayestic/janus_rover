#ifndef WEB_HANDLERS_H
#define WEB_HANDLERS_H
#include "Arduino.h"
#include <WiFi.h>
#include <atomic>

#include "../spi/spi_comm.h"
#include "../move_log/move_entry.h"
#include "../move_log/move_log.h"
#include "../sensor_data/sensor_data.h"

extern WiFiClient client;
extern WiFiClient live_client;
extern WiFiServer server;
extern String index_html;
extern String joystick_html;

extern SensorData currentSensorData;

extern std::atomic<bool> connected;
extern std::atomic<bool> uploadNeeded;
extern bool lowEnergyMode;
extern MoveLog moveLog;

extern bool joystickState;
extern String joystickType;
extern String moves;

void normalEnergy();
void lowEnergy();

extern SemaphoreHandle_t moveLogMutex;

void handleJoystickState(const String &req);

void handleStartPage();

void handleJoystickPage();

void handleDataRequest();

/* if request is "/" then client request the first page at root "/" -> it will return our site in index.h*/
void handleRootPage();

void handleVideoRequest();

void handleStreamStop();

void handleStreamStart();

void handleGoSleep();

void handleSendDataRequest();

void handleMoveResults();

void handleUnknownRequest(const String &req);

void ResponseToClientRequests(void *parameter); // Client from WEB

#endif