/*
  The rover consists two different boards. The first is ESP32 that acts as web server.
  It provides user's control of rover and can send emails, http requests, provide camera footage and etc.
  The second board is Arduino MEGA 2560 which is mounted directly onto rover's upper plate.
  This board handles the sensors (e.g. movement and temperature sensors) and steers the rover's wheel engines.
  Communication is provided via an SPI interface.
*/

// ── Core Arduino / ESP-IDF libraries ──────────────────────────
#include <Arduino.h>
#include <array>
#include <atomic>
#include <string.h>
#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <ESP_Mail_Client.h>
#include "esp_camera.h"
#include "esp_wifi.h"
#include "esp_bt.h"
#include "esp32-hal-cpu.h"
#include "esp_sntp.h"
#include "soc/soc.h" // disable brownout problems
#include "soc/rtc_cntl_reg.h"

// ── External content ──────────────────────────
#include "wifi_config.h"               //WiFi/mail secrets (gitignored)
#include "src/site_sources/index.h"    //main WWW site
#include "src/site_sources/joystick.h" //joystick steering WWW site

// ── Build / library config ────────────────────────────────────
SET_LOOP_TASK_STACK_SIZE(16384);
#define CONFIG_FREERTOS_PLACE_FUNCTIONS_INTO_FLASH
#define SILENT_MODE 1 // deifning silent mode fot mail sending client so it print less information

// ── Camera pins (AI-Thinker ESP32-CAM) ─────────────────────────
#define PWDN_GPIO_NUM 32
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

// ── SPI pins (communication with MEGA 2560) ─────────────────────────
#define HSPI_MISO 12
#define HSPI_MOSI 13
#define HSPI_SCLK 14
#define HSPI_SS 15

// ── SMTP config ─────────────────────────
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

// ── Global consts for capacity/protocols ─────────────────────────
#define MAX_REC_LEN 64            // max message length
#define MOVE_ENTRIES_CAPACITY 300 // max move entries stored

// ── Mail ─────────────────────────
SMTPSession smtp;
Session_Config mailConfig;

// ── NTP config ─────────────────────────
const char *ntpServer = "tempus1.gum.gov.pl";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 7200;

// ── Telemetry ─────────────────────────
// struct SensorData
// {
//   unsigned int currTemperature = 0;
//   unsigned int currHumidity = 0;
//   unsigned int currVoltage = 0;
// };

// SensorData currentSensorData{};

std::atomic<unsigned int> currTemperature{0};
std::atomic<unsigned int> currHumidity{0};
std::atomic<unsigned int> currVoltage{0};

// ── Web config ─────────────────────────
const int serverPort = server_port;
String serverIP = server_ip;     // Server IP that handles data,photos etc.
String serverPath = server_path; // Path on server
String joystickPath = "http://" + String(joystick_server_ip) + "/getJoyState";
String sendDataPath = "http://" + String(serverIP) + ":8080/sendData";
WiFiClient live_client;
WiFiClient client;
WiFiServer server(serverPort);

// ── Request type ─────────────────────────
enum class HttpRequestType
{
  Joystick,
  Telemetry
};

// ── Global flags ─────────────────────────
std::atomic<bool> uploadNeeded{false};
std::atomic<bool> connected{false};
bool gotMessage = false;
bool lowEnergyMode = false;

// ── SPI link ─────────────────────────
SemaphoreHandle_t spiMutex;
SPIClass *hspi = NULL;
String recivedData;

// ── WWW sites codes ─────────────────────────
String index_html = INDEX_page;
String joystick_html = JOYSTICK_page;

// ── Timings/watchdogd ─────────────────────────
unsigned long prevMillisLIVECAM = 0;
unsigned long boardStillAliveTimeout = 0;

// ── FreeRTOS task handlers ─────────────────────────
TaskHandle_t responses;
TaskHandle_t requests;

// ── Internal modules (are using extern variables from this file) ─────────────────────────
#include "src/spi/spi_comm.h"
#include "src/web/web_handlers.h"
#include "src/http/http_requests.h"
#include "src/move_log/move_entry.h"
#include "src/move_log/move_log.h"

// ── Move log ─────────────────────────
MoveLog moveLog;
char timeAll[24]; // last formatted timestamp
SemaphoreHandle_t moveLogMutex;

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

  cam_config.frame_size = FRAMESIZE_HD;
  cam_config.jpeg_quality = 16; // 0-63 lower number means higher quality
  cam_config.fb_count = 2;

  esp_err_t err = esp_camera_init(&cam_config);

  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  s->set_brightness(s, -1);
  s->set_hmirror(s, 1);

  if (s->id.PID == OV3660_PID)
  {

    s->set_brightness(s, 1);
    s->set_saturation(s, -1);
  }
}

void lowEnergy()
{
  // digitalWrite(33, HIGH);
  lowEnergyMode = true;
  send_data("lowEn");
  // WiFi.setSleep(true);
  esp_wifi_set_ps(wifi_ps_type_t::WIFI_PS_MAX_MODEM);
}

void normalEnergy()
{
  digitalWrite(33, LOW);
  lowEnergyMode = false;
  send_data("norEn");
  // WiFi.setSleep(false);
  esp_wifi_set_ps(wifi_ps_type_t::WIFI_PS_NONE);
}

void sendMailWithPhotos()
{
  xSemaphoreTake(moveLogMutex, portMAX_DELAY);
  if (moveLog.m_count < MOVE_ENTRIES_CAPACITY)
  {
    moveLog.m_count++;
  }
  xSemaphoreGive(moveLogMutex);

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
    xSemaphoreTake(moveLogMutex, portMAX_DELAY);
    if (!getLocalTime(&timeinfo))
    {
      Serial.println("Failed to obtain time");
      moveLog[moveLog.m_writeIndex] = "Failed";
      moveLog.m_writeIndex++;

      if (moveLog.m_writeIndex >= MOVE_ENTRIES_CAPACITY)
      {
        moveLog.m_writeIndex = 0;
      }
    }
    else if (i == 0)
    {
      strftime(timeAll, 18, "%m-%d %H:%M:%S", &timeinfo); // Load time
      moveLog[moveLog.m_writeIndex] = timeAll;
      moveLog.m_writeIndex++;

      if (moveLog.m_writeIndex >= MOVE_ENTRIES_CAPACITY)
      {
        moveLog.m_writeIndex = 0;
      }
    }
    xSemaphoreGive(moveLogMutex);

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
    httpDataRequest(HttpRequestType::Telemetry);
    uploadNeeded = false;
  }
}

void handleSPIRequests(void *parameter) // requests from MEGA 2560
{
  vTaskDelay(pdMS_TO_TICKS(1000));
  Serial.println("handleSPIRequests");

  for (;;)
  {
    vTaskDelay(pdMS_TO_TICKS(25));
    read_data();

    if (gotMessage == true)
    {
      String copyOfData = recivedData; // In case of recData = "" while operating on that string

      if (copyOfData == "MOVE")
      {
        Serial.println("MOVE DETECTED");
        // sendMailWithPhotos();
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
  // Serial.println("liveCam");
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

void setup()
{

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector

  setCpuFrequencyMhz(240);

  Serial.begin(115200);

  Serial.println("begin");

  pinMode(33, OUTPUT); // Set LED pinMode

  spiMutex = xSemaphoreCreateMutex();
  moveLogMutex = xSemaphoreCreateMutex();

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
          .listen_interval = 3,
      },
  };

  send_data("/0");

  WiFi.begin(wifi_ssid, wifi_password); // Connect to WiFi with primary values
  esp_wifi_set_ps(wifi_ps_type_t::WIFI_PS_NONE);
  WiFi.setAutoReconnect(true);

  int wifiTries = 0;

  WiFi.setSleep(false);

  delay(100);

  Serial.println(psramFound() ? "PSRAM: OK" : "PSRAM: BRAK");

  while (WiFi.status() != WL_CONNECTED)
  {
    if (wifiTries == 20)
    {
      break;
    }

    delay(1000);
    wifiTries++;
  }

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
  Serial.println(WiFi.status());

  delay(100);

  WiFi.disconnect();

  delay(200);

  WiFi.reconnect();

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

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  sntp_set_time_sync_notification_cb(timeSyncCallback);
  sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED); // szybka aktualizacja po pierwszym razie

  Serial.println("configNTP");

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, 1);

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

static void timeSyncCallback(struct timeval *tv)
{
  Serial.println("NTP: synchronized");
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(connected ? 5 : 30));

  if (millis() - boardStillAliveTimeout >= 700)
  {
    send_data("alv"); // alv so it is shorter and less time per sending
    boardStillAliveTimeout = millis();
  }

  if (connected)
  {
    if (millis() - prevMillisLIVECAM >= 55)
    {
      liveCam(live_client);
      prevMillisLIVECAM = millis();
    }
  }
}