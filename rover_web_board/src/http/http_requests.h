#ifndef HTTP_REQUESTS_H
#define HTTP_REQUESTS_H
#include "Arduino.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <atomic>
#include "../spi/spi_comm.h"

extern String joystickPath;
extern String sendDataPath;
extern std::atomic<unsigned int> currTemperature;
extern std::atomic<unsigned int> currHumidity;
extern std::atomic<unsigned int> currVoltage;

void httpDataRequest(HttpRequestType reqType)
{

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi Disconnected");
        return;
    }

    HTTPClient http;
    int httpCode = -1;

    if (reqType == HttpRequestType::Joystick)
    {
        Serial.println("HTTP GET joystick");
        http.begin(joystickPath.c_str());

        httpCode = http.GET();
        if (httpCode > 0)
        {
            Serial.printf("HTTP joystick Response code: %d\n", httpCode);
            String payload = http.getString();
            Serial.println(payload);
            send_data(payload);
        }
        else
        {
            Serial.printf("HTTP GET failed, code: %d\n", httpCode);
        }

        http.end();
        return;
    }

    if (reqType == HttpRequestType::Telemetry)
    {
        Serial.println("HTTP POST telemetry");
        http.begin(sendDataPath.c_str());
        http.addHeader("Content-Type", "application/json");

        String body = String("{\"temperature\":\"") + currTemperature + String("\",\"humidity\":\"") + currHumidity + String("\",\"voltage\":\"") + currVoltage + String("\"}");
        httpCode = http.POST(body);

        if (httpCode > 0)
        {
            Serial.printf("HTTP data Response code: %d\n", httpCode);
        }
        else
        {
            Serial.printf("HTTP POST failed, code: %d\n", httpCode);
        }

        http.end();
        return;
    }

    Serial.println("Unknown request type.");
}

#endif