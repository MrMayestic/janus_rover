/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-esp8266-web-server-http-authentication/

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

// Import required libraries
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "SPISlave.h"

#include "wifi_config.h"

const char *PARAM_INPUT_1 = "state";

const int output = 2;

String urll;
String jojko;
String joystickState;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Replaces placeholder with button section in your web page

void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);
  SPISlave.onData([](uint8_t *data, size_t len)
                  { joystickState = String((char *)data); });

  pinMode(output, OUTPUT);
  digitalWrite(output, LOW);

  // Connect to Wi-Fi
  WiFi.begin(wifi_ssid, wifi_password);

  int wifiTries = 20;

  while (WiFi.status() != WL_CONNECTED)
  {
    if (wifiTries == 20)
    {
      break;
    }

    Serial.println("Connecting to WiFi..");
    delay(500);
    // Serial.print(".");
    wifiTries++;
  }

  if (wifiTries >= 20)
  {
    WiFi.begin(wifi_ssid_reserve, wifi_password_reserve); // JEZELI SIE NIE UDALO DO POPRZEDNIEJ SIECI DO PROBA PODLACZENIA DO LOKALNEGO HOTSPOTA

    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      // Serial.print(".");
    }
    wifiTries = NULL;
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());
  SPISlave.begin();

  // Set the status register (if the master reads it, it will read this value)
  SPISlave.setStatus(millis());
  // Route for root / web page
  server.on("/getJoyState", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", joystickState.c_str()); });

  // Start server
  server.begin();
}

void loop()
{
}