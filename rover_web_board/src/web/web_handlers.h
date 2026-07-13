#ifndef WEB_HANDLERS_H
#define WEB_HANDLERS_H
#include "Arduino.h"
#include <WiFi.h>
#include <atomic>

#include "../spi/spi_comm.h"
#include "../move_log/move_entry.h"
#include "../move_log/move_log.h"

extern WiFiClient client;
extern WiFiClient live_client;
extern WiFiServer server;
extern String index_html;
extern String joystick_html;

extern std::atomic<unsigned int> currTemperature;
extern std::atomic<unsigned int> currHumidity;
extern std::atomic<unsigned int> currVoltage;

extern std::atomic<bool> connected;
extern std::atomic<bool> uploadNeeded;
extern bool lowEnergyMode;
extern MoveLog moveLog;

bool joystickState;
String joystickType;
String moves;

void normalEnergy();
void lowEnergy();

extern SemaphoreHandle_t moveLogMutex;

void handleJoystickState(const String &req)
{
    String joyBool = "";
    int joyStart = req.indexOf("k");

    for (int i = joyStart + 1; i <= req.length() - 1; i++)
    {
        joyBool = joyBool + String(req[i]);
    }

    if (joyBool == "True")
    {
        joystickState = true;
        joystickType = "phys";
    }
    else if (joyBool == "TrueWEB")
    {
        joystickState = true;
        joystickType = "web";
    }
    else
    {
        joystickState = false;
        joystickType = "";
    }
}

void handleStartPage()
{
    String httpMessageToClient = "HTTP/1.1 200 OK\n";
    httpMessageToClient += "Content-Type: text/html\n\n";
    httpMessageToClient += index_html;
    httpMessageToClient += "\n";

    client.print(httpMessageToClient);
}

void handleJoystickPage()
{
    String httpMessageToClient = "HTTP/1.1 200 OK\n";
    httpMessageToClient += "Content-Type: text/html\n\n";
    httpMessageToClient += joystick_html;
    httpMessageToClient += "\n";

    client.print(httpMessageToClient);
}

void handleDataRequest()
{
    String sendIt = "{\"temperature\":\"" + String(currTemperature) + "\",\"humidity\":\"" + String(currHumidity) + "\",\"voltage\":\"" + currVoltage + "\"}";

    String httpMessageToClient = "HTTP/1.1 200 OK\n";
    httpMessageToClient += "Access-Control-Allow-Headers: *\n";
    httpMessageToClient += "Access-Control-Allow-Origin: *\n";
    httpMessageToClient += "Content-Type: application/json\n\n";
    httpMessageToClient += sendIt;
    httpMessageToClient += "\n";

    client.print(httpMessageToClient);
}

/* if request is "/" then client request the first page at root "/" -> it will return our site in index.h*/
void handleRootPage()
{
    String httpMessageToClient = "HTTP/1.1 200 OK\n";
    httpMessageToClient += "Content-Type: text/html\n\n";
    httpMessageToClient += index_html;
    httpMessageToClient += "\n";

    client.print(httpMessageToClient);
}

void handleVideoRequest()
{
    live_client = client;

    live_client.print("HTTP/1.1 200 OK\n");
    live_client.print("Access-Control-Allow-Headers: *\n");
    live_client.print("Access-Control-Allow-Origin: *\n");
    live_client.print("Content-Type: multipart/x-mixed-replace; boundary=frame\n\n");
    live_client.flush();

    connected = true;
}

void handleStreamStop()
{
    client.stop();
    connected = false;
}

void handleStreamStart()
{
    live_client.flush();
    connected = true;
}

void handleGoSleep()
{
    Serial.println("Going to sleep now");

    delay(500);

    esp_deep_sleep_start();
}

void handleSendDataRequest()
{
    send_data("sendData");
    uploadNeeded = true;
}

void handleMoveResults()
{
    xSemaphoreTake(moveLogMutex, portMAX_DELAY);

    moves = "{\"data\":\"";

    for (int i = 0; i < moveLog.m_count; i++)
    {
        moves += moveLog[i].m_timestamp.data();
    }

    moves += "\"}";

    xSemaphoreGive(moveLogMutex);

    String httpMessageToClient = "HTTP/1.1 200 OK\n";
    httpMessageToClient += "Access-Control-Allow-Headers: *\n";
    httpMessageToClient += "Access-Control-Allow-Origin: *\n";
    httpMessageToClient += "Content-Type: application/json\n\n";
    httpMessageToClient += moves;
    httpMessageToClient += "\n";

    client.print(httpMessageToClient);

    delay(100);

    moves = "";
}

void handleUnknownRequest(const String &req)
{
    if (req != "/favicon.ico")
    {
        send_data(req);
    }
}

void ResponseToClientRequests(void *parameter) // Client from WEB
{
    vTaskDelay(pdMS_TO_TICKS(2000));
    Serial.println("ReplyToClientRequests");
    for (;;)
    {
        // Serial.println("WEB_HANDLERS");
        vTaskDelay(pdMS_TO_TICKS(10));
        client = server.available();
        if (!client)
        {
            vTaskDelay(pdMS_TO_TICKS(200));
            continue; // żaden klient nie czeka -> powtórz pętlę
        }
        /* check client is connected */
        if (client.connected())
        {
            // Serial.println("CLIENT");
            /* client send request? */
            /* request end with '\r' -> this is HTTP protocol format */
            String req = "";

            while (client.available())
            {
                req += (char)client.read();
            }

            /* First line of HTTP request is "GET / HTTP/1.1"
              here "GET /" is a request to get the first page at root "/"
              "HTTP/1.1" is HTTP version 1.1
            */
            /* now we parse the request to see which page the client want */
            int addr_start;

            if (req.indexOf("OPTIONS") != -1)
            {
                addr_start = req.indexOf("OPTIONS") + strlen("OPTIONS");
            }
            else
            {
                addr_start = req.indexOf("GET") + strlen("GET");
            }

            int addr_end = req.indexOf("HTTP", addr_start);

            if (addr_start == -1 || addr_end == -1)
            {
                continue;
            }

            req = req.substring(addr_start, addr_end);
            req.trim();

            Serial.print("Request: ");
            Serial.println(req);

            if (!lowEnergyMode)
            {
                digitalWrite(33, HIGH);
            }

            if (req.indexOf("joy") != -1 && req != "/joystickPage")
            {
                handleJoystickState(req);
            }
            else if (req.indexOf("Page") != -1)
            {
                if (req == "/startPage")
                {
                    handleStartPage();
                }
                else if (req == "/joystickPage")
                {
                    handleJoystickPage();
                }
            }
            else if (req == "/data")
            {
                handleDataRequest();
            }
            else if (req == "/")
            {
                handleRootPage();
            }
            else if (req == "/video")
            {
                handleVideoRequest();
            }
            else if (req == "/streamStop")
            {
                handleStreamStop();
            }
            else if (req == "/streamStart")
            {
                handleStreamStart();
            }
            else if (req == "/gosleep")
            {
                handleGoSleep();
            }
            else if (req == "/sendData")
            {
                handleSendDataRequest();
            }
            else if (req == "/normalEnergy")
            {
                normalEnergy();
            }
            else if (req == "/lowEnergy")
            {
                lowEnergy();
            }
            else if (req == "/moveResults")
            {
                handleMoveResults();
            }
            else
            {
                handleUnknownRequest(req);
            }
        }
        if (!lowEnergyMode)
        {
            digitalWrite(33, LOW);
        }
    }
}

#endif