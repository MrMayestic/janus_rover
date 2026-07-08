#ifndef WEB_HANDLERS_H
#define WEB_HANDLERS_H
#include "Arduino.h"
#include <WiFi.h>

#include "../spi/spi_comm.h"
#include "../move_log/move_entry.h"
#include "../move_log/move_log.h"

extern WiFiClient client;
extern WiFiClient live_client;
extern WiFiServer server;
extern String index_html;
extern String joystick_html;

extern unsigned int currTemperature;
extern unsigned int currHumidity;
extern unsigned int currVoltage;

extern bool connected;
extern bool uploadNeeded;
extern bool lowEnergyMode;
extern MoveLog moveLog;

String moves;
bool canVideo;
bool canLoad;
bool joystickState;
String joystickType;

void normalEnergy();

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

            String httpMessageToClient = "";

            if (req.indexOf("joy") != -1 && req != "/joystickPage")
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
            else if (req.indexOf("Page") != -1)
            {
                if (req == "/startPage")
                {
                    httpMessageToClient = "HTTP/1.1 200 OK\n";
                    httpMessageToClient += "Content-Type: text/html\n\n";
                    httpMessageToClient += index_html;
                    httpMessageToClient += "\n";

                    client.print(httpMessageToClient);
                }
                else if (req == "/joystickPage")
                {
                    httpMessageToClient = "HTTP/1.1 200 OK\n";
                    httpMessageToClient += "Content-Type: text/html\n\n";
                    httpMessageToClient += joystick_html;
                    httpMessageToClient += "\n";

                    client.print(httpMessageToClient);
                }
            }
            else if (req == "/data")
            {
                String sendIt = "{\"temperature\":\"" + String(currTemperature) + "\",\"humidity\":\"" + String(currHumidity) + "\",\"voltage\":\"" + currVoltage + "\"}";

                httpMessageToClient = "HTTP/1.1 200 OK\n";
                httpMessageToClient += "Access-Control-Allow-Headers: *\n";
                httpMessageToClient += "Access-Control-Allow-Origin: *\n";
                httpMessageToClient += "Content-Type: application/json\n\n";
                httpMessageToClient += sendIt;
                httpMessageToClient += "\n";

                client.print(httpMessageToClient);
            }

            /* if request is "/" then client request the first page at root "/" -> it will return our site in index.h*/

            else if (req == "/")
            {
                httpMessageToClient = "HTTP/1.1 200 OK\n";
                httpMessageToClient += "Content-Type: text/html\n\n";
                httpMessageToClient += index_html;
                httpMessageToClient += "\n";

                client.print(httpMessageToClient);

                if (canLoad == true)
                {
                    live_client = client;
                    live_client.print("HTTP/1.1 200 OK\n");
                    live_client.print("Access-Control-Allow-Origin: *\n");
                    live_client.print("Content-Type: multipart/x-mixed-replace; boundary=frame\n\n");
                    live_client.flush();

                    canVideo = true;
                    canLoad = false;
                }
                else
                {
                    canVideo = true;
                }

                digitalWrite(CONTROL_PIN_NUM, HIGH);
            }

            else if (req == "/video")
            {

                live_client = client;

                live_client.print("HTTP/1.1 200 OK\n");
                live_client.print("Access-Control-Allow-Headers: *\n");
                live_client.print("Access-Control-Allow-Origin: *\n");
                live_client.print("Content-Type: multipart/x-mixed-replace; boundary=frame\n\n");
                live_client.flush();

                connected = true;

                if (canVideo == true)
                {
                    // Manually (request from site after manual click by user) load of video
                }
                else
                {
                    canLoad = true;
                }
            }
            else if (req == "/streamStop")
            {
                client.stop();
                connected = false;
            }
            else if (req == "/streamStart")
            {
                live_client.flush();
                connected = true;
            }
            else if (req == "/gosleep")
            {
                Serial.println("Going to sleep now");

                delay(500);

                esp_deep_sleep_start();
            }
            else if (req == "/sendData")
            {
                send_data("sendData");
                uploadNeeded = true;
            }
            else if (req == "/normalEnergy")
            {
                normalEnergy();
            }
            else if (req == "/moveResults")
            {
                moves = "{\"data\":\"";

                for (int i = 0; i < moveLog.m_count; i++)
                {
                    moves += moveLog[i].m_timestamp.data();
                }

                moves += "\"}";

                httpMessageToClient = "HTTP/1.1 200 OK\n";
                httpMessageToClient += "Access-Control-Allow-Headers: *\n";
                httpMessageToClient += "Access-Control-Allow-Origin: *\n";
                httpMessageToClient += "Content-Type: application/json\n\n";
                httpMessageToClient += moves;
                httpMessageToClient += "\n";

                client.print(httpMessageToClient);

                delay(100);

                moves = "";
            }
            else
            {
                if (req != "/favicon.ico")
                {
                    send_data(req);
                }
            }
        }
        if (!lowEnergyMode)
        {
            digitalWrite(33, LOW);
        }
    }
}

#endif