/*
  The rover consists two different boards. The first is ESP32 that acts as web server.
  It provides user's control of rover and can send emails, http requests, provide camera footage and etc.
  The second board is Arduino MEGA 2560 which is mounted directly onto rover's upper plate.
  This board handles the sensors (e.g. movement and temperature sensors) and steers the rover's wheel engines.
  Communication is provided via an SPI interface.
*/

#include <SPI.h>
#include <Arduino.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <string.h>
#include <WiFi.h>

#include "esp_camera.h"
#include "esp_wifi.h"
#include "esp_bt.h"
#include "esp32-hal-cpu.h"
#include "esp_sntp.h"

#include "soc/soc.h" // disable brownout problems
#include "soc/rtc_cntl_reg.h"

#include "index.h"
#include "joystick.h"

#include "wifi_config.h"

#include "SPIFFS.h"

#include <ESP_Mail_Client.h>

SET_LOOP_TASK_STACK_SIZE(16384);

#define CONFIG_FREERTOS_PLACE_FUNCTIONS_INTO_FLASH
#define SILENT_MODE 1 // deifning silent mode fot mail sending client so it print less information

#define PWDN_GPIO_NUM 32 // Pins definition to handle ESP32 camera
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

#define HSPI_MISO 12
#define HSPI_MOSI 13
#define HSPI_SCLK 14
#define HSPI_SS 15

#define CONTROL_PIN_NUM 2
#define FLASH_GPIO_NUM 4

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

#define MAX_REC_LEN 64 // max message length

// Declare the global used SMTPSession object for SMTP transport
SMTPSession smtp;

// Declare the global used Session_Config for user defined session credentials
Session_Config mailConfig;

/*Start defining variables*/

String moveTimes[342];

char timeAll[24];

unsigned int moveCounter = 0;

String moves = "";

const char *ntpServer = "tempus1.gum.gov.pl";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 7200;

unsigned int currTemperature = 0;
unsigned int currHumidity = 0;
unsigned int currVoltage = 0;

const int serverPort = server_port; // Server port

static const int spiClk = 4000000; // Clock for SPI

unsigned long prevMillisLIVECAM = 0;
unsigned long boardStillAliveTimeout = 0;

/*
Above timeout is security feature for keeping informed the MEGA 2560 board if ESP32 is or isn't active
(mainly to shut engines down if esp32 restarts and rover is moving)
*/

bool canVideo = false;
bool canLoad = false;

bool uploadNeeded = false;

bool connected = false;

bool gotMessage = false;

bool joystickState = false;

bool lowEnergyMode = false;

String joystickType = "";

String serverIP = server_ip;     // Server IP that handles data,photos etc.
String serverPath = server_path; // Path on server

String recivedData;

String joystickPath = "http://" + String(joystick_server_ip) + "/getJoyState";
String sendDataPath = "http://" + String(serverIP) + ":8080/sendData";

String index_html = INDEX_page;       // Load index site (HTML,CSS,JS)
String joystick_html = JOYSTICK_page; // Load joystick site (HTML,CSS,JS)

WiFiClient live_client;
WiFiClient client;
WiFiServer server(serverPort);

SemaphoreHandle_t spiMutex;
SPIClass *hspi = NULL;

TaskHandle_t responses;
TaskHandle_t requests;

void setup()
{

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector

  setCpuFrequencyMhz(240);

  Serial.begin(115200);

  Serial.println("begin");

  pinMode(33, OUTPUT); // Set LED pinMode

  pinMode(CONTROL_PIN_NUM, OUTPUT);

  spiMutex = xSemaphoreCreateMutex();

  if (!spiMutex)
  {
    Serial.println("Nie można utworzyć muteksu SPI");
    while (true)
      vTaskDelay(pdMS_TO_TICKS(1000));
  }

  Serial.println("mutex");

  hspi = new SPIClass(HSPI);

  if (!SPIFFS.begin(true))
  {
    // Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  pinMode(HSPI_SS, OUTPUT);
  digitalWrite(HSPI_SS, HIGH);

  hspi->begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_SS);

  Serial.println("SPI");

  wifi_config_t wifi_config = {
      .sta = {
          .listen_interval = 7,
      },
  };

  send_data("/0");

  WiFi.begin(wifi_ssid, wifi_password); // Connect to WiFi with primary values

  WiFi.setAutoReconnect(true);

  int wifiTries = 0;

  WiFi.setSleep(false);

  esp_wifi_set_ps(wifi_ps_type_t::WIFI_PS_NONE);

  delay(100);

  while (WiFi.status() != WL_CONNECTED)
  {
    if (wifiTries == 20)
    {
      break;
    }

    delay(1000);
    wifiTries++;
  }

  delay(100);

  // If tries of connecting to primary WiFi are >= 20 then program tries to connect to secondary WiFi beacuse primary is probably not working

  if (wifiTries >= 20)
  {
    WiFi.begin(wifi_ssid_reserve, wifi_password_reserve);

    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
    }

    wifiTries = NULL;
  }

  Serial.println(WiFi.localIP());
  Serial.println("wifi");

  mailConfig.server.host_name = SMTP_HOST;     // for outlook.com
  mailConfig.server.port = SMTP_PORT;          // for TLS with STARTTLS or 25 (Plain/TLS with STARTTLS) or 465 (SSL)
  mailConfig.login.email = AUTHOR_EMAIL;       // set to empty for no SMTP Authentication
  mailConfig.login.password = AUTHOR_PASSWORD; // set to empty for no SMTP Authentication

  // For client identity, assign invalid string can cause server rejection
  mailConfig.login.user_domain = "";

  smtp.debug(1);

  String IP = WiFi.localIP().toString();

  index_html.replace("change_this_ip", IP);
  index_html.replace("index.html", "startPage");
  index_html.replace("joystick.html", "joystickPage");

  joystick_html.replace("change_this_ip", IP);
  joystick_html.replace("index.html", "startPage");
  joystick_html.replace("joystick.html", "joystickPage");

  Serial.println("configNTP");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  sntp_set_time_sync_notification_cb(timeSyncCallback);
  sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED); // szybka aktualizacja po pierwszym razie

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, 1);

  // Power saving
  esp_bt_controller_disable(); // disable bluetooth for power saving

  delay(10);
  configCamera();
  delay(20);
  server.begin();

  // lowEnergy();

  xTaskCreatePinnedToCore(
      handleSPIRequests, /* Task function. */
      "requests",        /* name of task. */
      10000,             /* Stack size of task */
      NULL,              /* parameter of the task */
      1,                 /* priority of the task */
      &requests,         /* Task handle to keep track of created task */
      1);                /* pin task to core 1 */

  Serial.println("request task");

  xTaskCreatePinnedToCore(
      ResponseToClientRequests, /* Task function. */
      "responses",              /* name of task. */
      10000,                    /* Stack size of task */
      NULL,                     /* parameter of the task */
      1,                        /* priority of the task */
      &responses,               /* Task handle to keep track of created task */
      0);                       /* pin task to core 0 */

  Serial.println("Setup done.");
}

void configCamera()
{
  camera_config_t cam_config;
  cam_config.ledc_channel = LEDC_CHANNEL_0;
  cam_config.ledc_timer = LEDC_TIMER_0;
  cam_config.pin_d0 = Y2_GPIO_NUM;
  cam_config.pin_d1 = Y3_GPIO_NUM;
  cam_config.pin_d2 = Y4_GPIO_NUM;
  cam_config.pin_d3 = Y5_GPIO_NUM;
  cam_config.pin_d4 = Y6_GPIO_NUM;
  cam_config.pin_d5 = Y7_GPIO_NUM;
  cam_config.pin_d6 = Y8_GPIO_NUM;
  cam_config.pin_d7 = Y9_GPIO_NUM;
  cam_config.pin_xclk = XCLK_GPIO_NUM;
  cam_config.pin_pclk = PCLK_GPIO_NUM;
  cam_config.pin_vsync = VSYNC_GPIO_NUM;
  cam_config.pin_href = HREF_GPIO_NUM;
  cam_config.pin_sscb_sda = SIOD_GPIO_NUM;
  cam_config.pin_sscb_scl = SIOC_GPIO_NUM;
  cam_config.pin_pwdn = PWDN_GPIO_NUM;
  cam_config.pin_reset = RESET_GPIO_NUM;
  cam_config.xclk_freq_hz = 20000000;
  cam_config.pixel_format = PIXFORMAT_JPEG;

  cam_config.frame_size = FRAMESIZE_SVGA;
  cam_config.jpeg_quality = 5; // 0-63 lower number means higher quality
  cam_config.fb_count = 2;

  esp_err_t err = esp_camera_init(&cam_config);

  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
}

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
    delayMicroseconds(20);
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
      Serial.print(char(byteRead));
    }
    delayMicroseconds(25);
  }

  digitalWrite(HSPI_SS, HIGH);
  hspi->endTransaction();

  xSemaphoreGive(spiMutex);
}

void lowEnergy()
{
  // digitalWrite(33, HIGH);
  lowEnergyMode = true;
  send_data("lowEn");
  WiFi.setSleep(true);
  esp_wifi_set_ps(wifi_ps_type_t::WIFI_PS_MAX_MODEM);
}

void normalEnergy()
{
  digitalWrite(33, LOW);
  lowEnergyMode = false;
  send_data("norEn");
  WiFi.setSleep(false);
  esp_wifi_set_ps(wifi_ps_type_t::WIFI_PS_NONE);
}

static void timeSyncCallback(struct timeval *tv)
{
  Serial.println("NTP: synchronized");
}

/* Codes
1 - joystick
10 - send POST req with measured data
*/

void httpDataRequest(int whichReq)
{

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi Disconnected");
    return;
  }

  HTTPClient http;
  int httpCode = -1;

  if (whichReq == 1)
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

  if (whichReq == 10)
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

  Serial.printf("Unknown request type: %d\n", whichReq);
}

void ResponseToClientRequests(void *parameter) // Client from WEB
{
  vTaskDelay(pdMS_TO_TICKS(2000));
  Serial.println("ReplyToClientRequests");
  for (;;)
  {
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

        for (int i = 0; i < 340; i++)
        {
          moves += moveTimes[i];
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

void sendMailWithPhotos()
{
  memset(timeAll, 0, sizeof(timeAll) / sizeof(timeAll[0]));

  SMTP_Message message;

  for (int i = 0; i <= 1; i++)
  {
    SMTP_Attachment att;

    bool wasConnected = connected;

    if (!connected)
    {
      client.stop();
      connected = false;
    }
    vTaskDelay(pdMS_TO_TICKS(50));

    camera_fb_t *photo = esp_camera_fb_get();

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
      Serial.println("Failed to obtain time");
      moveTimes[moveCounter] = String("Failed") + String("|");
      moveCounter++;

      if (moveCounter >= 340)
      {
        moveCounter = 0;
      }
    }
    else if (i == 0)
    {
      strftime(timeAll, 18, "%m-%d %H:%M:%S", &timeinfo); // Load time
      moveTimes[moveCounter] = String(timeAll) + String("|");
      moveCounter++;

      if (moveCounter >= 340)
      {
        moveCounter = 0;
      }
    }

    if (photo)
    {
      uint8_t *photoBuf = photo->buf;

      // Set the attatchment info

      att.descr.filename = String(timeAll) + ".jpg";
      att.descr.mime = "image/jpeg";
      att.blob.data = photoBuf;
      att.blob.size = photo->len;
      // Set the transfer encoding to base64
      att.descr.transfer_encoding = Content_Transfer_Encoding::enc_base64;
      // // We set the content encoding to match the above greenImage data
      // att.descr.content_encoding = Content_Transfer_Encoding::enc_base64;

      // Add attachment to the message
      message.addAttachment(att);

      esp_camera_fb_return(photo);

      vTaskDelay(pdMS_TO_TICKS(25));

      if (wasConnected)
      {
        live_client.flush();
        connected = true;
      }
    }

    vTaskDelay(pdMS_TO_TICKS(900));
  }

  message.sender.name = "Janus_rover";
  message.sender.email = RECIPIENT_EMAIL;
  message.subject = "Move " + String(timeAll);
  message.addRecipient("name1", RECIPIENT_EMAIL);

  message.text.content = String(timeAll);

  smtp.connect(&mailConfig);

  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());
}

void sendAirAndVoltageData(String recivedData)
{
  int tempStartingIndex = recivedData.indexOf("t");
  int humidityStartingIndex = recivedData.indexOf("h");
  int voltageStartingIndex = recivedData.indexOf("v");
  String temperature = recivedData.substring(tempStartingIndex + 1, humidityStartingIndex);
  String humidity = recivedData.substring(humidityStartingIndex + 1, voltageStartingIndex);
  String voltage_read = recivedData.substring(voltageStartingIndex + 1, recivedData.length());

  recivedData = "";

  if (temperature.toInt() > 0 && temperature.toInt() < 50)
  {
    currTemperature = temperature.toInt();
  }
  if (humidity.toInt() > 0 && humidity.toInt() <= 100)
  {
    currHumidity = humidity.toInt();
  }
  if (voltage_read.toInt() > 0)
  {
    currVoltage = voltage_read.toInt();
  }

  if (uploadNeeded)
  {
    httpDataRequest(10);
    uploadNeeded = false;
  }
}

void handleSPIRequests(void *parameter) // requests from second board, MEGA 2560
{
  vTaskDelay(pdMS_TO_TICKS(1000));
  Serial.println("handleSPIRequests");

  for (;;)
  {
    vTaskDelay(pdMS_TO_TICKS(8));
    read_data();

    if (gotMessage == true)
    {
      String copyOfData = recivedData; // In case of recData = "" while operating on that string

      if (copyOfData == "MOVE")
      {
        sendMailWithPhotos();
      }
      else
      {
        sendAirAndVoltageData(copyOfData);
      }

      gotMessage = false;
      recivedData = "";
    }
  }
}

void liveCam(WiFiClient &client)
{

  camera_fb_t *fb = esp_camera_fb_get();

  if (!fb)
  {
    Serial.println("Frame buffer could not be acquired");
    return;
  }
  String liveS = "--frame\n";
  liveS += "Content-Type: image/jpeg\n\n";
  client.print(liveS);
  client.flush();
  client.write(fb->buf, fb->len);
  client.flush();
  client.print("\n");
  // return the frame buffer back to be reused
  esp_camera_fb_return(fb);
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(2));

  if (millis() - boardStillAliveTimeout >= 700)
  {
    send_data("alv"); //alv so it is shorter and less time per sending
    boardStillAliveTimeout = millis();
  }

  if (connected == true)
  {
    if (millis() - prevMillisLIVECAM >= 25)
    {
      liveCam(live_client);
      prevMillisLIVECAM = millis();
    }
  }
}