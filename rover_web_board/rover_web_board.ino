#include "esp_camera.h" //Including files, libraries, other files
#include <SPI.h>
#include <Arduino.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include "esp_wifi.h"
// #include <WiFi.h>
#include "soc/soc.h" // disable brownout problems
#include "soc/rtc_cntl_reg.h"

#include "index.h"

#include "wifi_config.h"
// #include "style.css"
#include "SPIFFS.h"
#include "time.h"
#include <string.h>

#define RXp2 3
#define TXp2 1

const char *ntpServer = "vega.cbk.poznan.pl"; // Set time server
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 7200;

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

// #define PRZOD 12
// #define LEWO 13
// #define TYL 15
// #define PRAWO 14
// #define WAKEUP 2

bool canVideo = false;
bool canLoad = false;
bool canUpload = true;

// int serialSendData;

static const int spiClk = 2000000; // 2 MHz

String serverIP = server_ip;     // IP SERWERA (LOKALNY SERWER CZYLI KOMPUTER Z PROGRAMEM XAMPP)
String serverPath = server_path; // ŚCIEŻKA DO OBSLUGI LAZIKA

const int serverPort = server_port; // PORT SERWERA
WiFiServer server(serverPort);      // USTAWIENIE SERWERA

bool connected = true;

WiFiClient live_client; // USTAWIENIE KLASY CLIENT
WiFiClient client;

SPIClass *hspi = NULL;

String s;

String sendData;

bool joystickState = false;

String joystickPath = "http://" + String(joystick_server_ip) + "/getJoyState";

int httpResponseCode;

String payload;

String index_html = MAIN_page; // Load client site (HTML,CSS,JS)

unsigned long lastTime = 0;

unsigned long timerDelay = 50;

// Configure camera in ESP32

void configCamera()
{
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 5; // 0-63 lower number means higher quality
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    // Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void liveCam(WiFiClient &client)
{ // SKRYPT DO PRZECHWYCENIA ZDJECIA Z KAMERY I WCZYTANIA DO STRONY

  // capture a frame
  camera_fb_t *fb = esp_camera_fb_get();

  if (!fb)
  {
    // Serial.println("Frame buffer could not be acquired");
    return;
  }

  client.print("--frame\n");
  client.print("Content-Type: image/jpeg\n\n");
  client.flush();
  client.write(fb->buf, fb->len);
  client.flush();
  client.print("\n");
  // return the frame buffer back to be reused
  esp_camera_fb_return(fb);
}

// Setup function

void setup()
{

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector

  Serial.begin(115200);

  pinMode(33, OUTPUT); // USTAWIENIE BLINKa

  hspi = new SPIClass(HSPI);

  if (!SPIFFS.begin(true))
  {
    // Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // File root = SPIFFS.open("/");

  // File file = root.openNextFile();

  // while (file)
  // {

  //   // Serial.print("FILE: ");
  //   // Serial.println(file.name());

  //   file = root.openNextFile();
  // }

  WiFi.begin(wifi_ssid, wifi_password); // Connect to WiFi with primary values
  // Serial.println("");
  int wifiTries = 0;
  // WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED)
  { // SPRAWDZENIE CZY PODLACZONOSIE DO SIECI
    if (wifiTries == 20)
    {
      break;
    }

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

  // Serial.println("");
  String IP = WiFi.localIP().toString();

  index_html.replace("server_ip", IP);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); // Time set
  //   printLocalTime();
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, 1);

  server.begin();

  configCamera();

  hspi->begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_SS);

  pinMode(HSPI_SS, OUTPUT);
  // Serial2.begin(19200, SERIAL_8N1, RXp2, TXp2);
}

//--------------------------------------------------------------------

void hspiCommand(String stringMess) // SPI TRANSFER FUNCTION
{
  Serial.println(stringMess);
  Serial.println(" przesylam");

  char buff[stringMess.length() + 1];
  // Serial.println(message);
  // uint8_t *data = (uint8_t *)message;
  stringMess.toCharArray(buff, stringMess.length() + 1);
  // Serial.println(message);
  // byte stuff = send.toInt();
  hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));

  digitalWrite(HSPI_SS, LOW);

  for (int i = 0; i < sizeof buff; i++)
  { /* transfer buff data per second */
    // Serial.print(buff[i]);
    // delayMicroseconds(10);
    hspi->transfer(buff[i]);
  }
  // Serial.println(" ");

  digitalWrite(HSPI_SS, HIGH);

  hspi->endTransaction();
}

/*Function that handles http requests and responses*/

void http_resp()
{
  // delay(5);

  // Serial.println(server.available());
  /* check client is connected */
  if (client.connected())
  {

    /* client send request? */
    /* request end with '\r' -> this is HTTP protocol format */
    String req = "";

    while (client.available())
    {
      // Serial.println("elo");
      req += (char)client.read();
    }
    // Serial.println("request " + req);

    /* First line of HTTP request is "GET / HTTP/1.1"
      here "GET /" is a request to get the first page at root "/"
      "HTTP/1.1" is HTTP version 1.1
    */
    /* now we parse the request to see which page the client want */
    // client.flush();
    // Serial.println(req);
    int addr_start;
    if (req.indexOf("GET") != -1)
    {
      addr_start = req.indexOf("GET") + strlen("GET");
    }
    else
    {
      addr_start = req.indexOf("HEAD") + strlen("HEAD");
    }

    int addr_end = req.indexOf("HTTP", addr_start);

    if (addr_start == -1 || addr_end == -1)
    {
      // Serial.println("Invalid request " + req);
      return;
    }

    req = req.substring(addr_start, addr_end);
    req.trim();

    digitalWrite(33, HIGH);

    Serial.println("Request: " + req);
    s = "";

    if (req.indexOf("joy") != -1)
    {
      // Serial.println("kuwa");
      String joyBool = "";
      int joyStart = req.indexOf("k");

      for (int i = joyStart + 1; i <= req.length() - 1; i++)
      {
        joyBool = joyBool + req[i];
      }

      Serial.println(joyBool);

      if (joyBool == "True")
      {
        joystickState = true;
        Serial.println("TruJes");
      }
      else
      {
        joystickState = false;
      }
    }

    /* if request is "/" then client request the first page at root "/" -> it will return our site in index.h*/
    else if (req == "/") // IFs to handle requests
    {
      // Serial.println("bróh");
      delay(15);

      s = "HTTP/1.1 200 OK\n";
      s += "Content-Type: text/html\n\n";
      s += index_html;
      s += "\n";

      client.print(s);
      // Serial.println(canLoad);
      // Serial.println(canVideo);
      delay(100);

      if (canLoad == true)
      {
        // Serial.println("VIDEO with SLASH"); //Load video on site
        live_client = client;
        live_client.print("HTTP/1.1 200 OK\n");
        live_client.print("Access-Control-Allow-Origin: *\n");
        live_client.print("Content-Type: multipart/x-mixed-replace; boundary=frame\n\n");
        live_client.flush();
        canUpload = true;
        connected = true;
        canVideo = true;
        canLoad = false;
      }
      else
      {
        canVideo = true;
      }
      //  client.stop();
    }

    else if (req == "/video")
    {
      delay(20);

      if (canVideo == true)
      {
        // Serial.println("VIDEO"); //Manually (request from site after manual click by user) load of video

        live_client = client;

        live_client.print("HTTP/1.1 200 OK\n");
        live_client.print("Access-Control-Allow-Headers: *\n");
        live_client.print("Access-Control-Allow-Origin: *\n");
        live_client.print("Content-Type: multipart/x-mixed-replace; boundary=frame\n\n");
        live_client.flush();

        canUpload = true;
        connected = true;
      }
      else
      {
        canLoad = true;
      }
    }
    else if (req == "/stop") // Stream off
    {
      client.stop();
      connected = false;
    }
    else if (req == "/start") // Stream on
    {
      // client.flush();
      connected = true;
      live_client.flush();
    }
    else if (req == "/gosleep")
    {
      // Serial.println("Going to sleep now");
      //  digitalWrite(WAKEUP,HIGH);
      // digitalWrite(PRZOD,LOW);
      //  digitalWrite(LEWO,LOW);
      //  digitalWrite(TYL,LOW);
      //  digitalWrite(PRAWO,LOW);
      // delay(100);
      // digitalWrite(WAKEUP,LOW);
      //  pinMode(WAKEUP,INPUT);
      delay(500);
      esp_deep_sleep_start();
    }
    else
    {
      if (req != "/favicon.ico")
        hspiCommand(req + "\n");
    }
  }
  digitalWrite(33, LOW);
}

//---------------------HTTP request functions (to send joystick requests to web_board of joystick)---------------------------------------------------------------

void http_request()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;

    // Your Domain name with URL path or IP address with path
    http.begin(joystickPath.c_str());

    // Send HTTP GET request
    httpResponseCode = http.GET();

    if (httpResponseCode > 0)
    {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      payload = http.getString();

      Serial.println(payload);

      sendData = payload + "\n";
      hspiCommand(sendData);
    }
    else
    {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  }
  else
  {
    Serial.println("WiFi Disconnected");
  }
}

//---------------------LOOP FUNCTION----------------------------------------------------------------------------

void loop()
{ // LOOP
  client = server.available();

  if (((millis() - lastTime) > timerDelay) && joystickState)
  {
    // Check WiFi connection status
    http_request();
    lastTime = millis();
  }
  // Serial.println(ESP.getFreeHeap());
  http_resp();
  if (connected == true)
  {
    // //Serial.println("bruh");
    liveCam(live_client);
  }
}

// Function to send photo from camera to server

String sendPhoto()
{
  // WiFiClient client;
  String getAll;
  String getBody;

  camera_fb_t *fb = NULL;
  fb = esp_camera_fb_get(); // Getting image

  if (!fb)
  {
    // Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }

  // Serial.println("Connecting to server: " + serverIP);

  if (live_client.connect(serverIP.c_str(), serverPort))
  {

    struct tm timeinfo;

    if (!getLocalTime(&timeinfo))
    {
      // Serial.println("Failed to obtain time");
    }
    // Serial.println(&timeinfo,"%Y%m%d");
    char timeAll[24];
    strftime(timeAll, 20, "%Y%m%d_%H%M%S", &timeinfo); // Load time

    // Serial.println(timeAll);
    // Serial.println(timeAll[20]);
    // Serial.println("Connection successful!");
    String head = "--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"imageFile\"; filename=\"esp32photo_" + String(timeAll) + ".jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--RandomNerdTutorials--\r\n";

    uint32_t imageLen = fb->len;
    uint32_t extraLen = head.length() + tail.length();
    uint32_t totalLen = imageLen + extraLen;

    live_client.println("POST " + serverPath + " HTTP/1.1");
    live_client.println("Host: " + serverIP);
    live_client.println("Content-Length: " + String(totalLen));
    live_client.println("Content-Type: multipart/form-data; boundary=RandomNerdTutorials"); // Creating first headers in HTTP request
    live_client.println();
    live_client.print(head);

    uint8_t *fbBuf = fb->buf;

    size_t fbLen = fb->len;
    // Serial.println(fbLen);

    for (size_t n = 0; n < fbLen; n = n + 1024) // Start sending image
    {
      if (n + 1024 < fbLen)
      {
        live_client.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen % 1024 > 0)
      {
        size_t remainder = fbLen % 1024;
        live_client.write(fbBuf, remainder);
      }
    }
    // Serial.println("bruh!");
    live_client.print(tail);

    esp_camera_fb_return(fb);

    int timoutTimer = 10000;
    long startTimer = millis();

    boolean state = false;

    while ((startTimer + timoutTimer) > millis())
    {
      // Serial.print(".");
      delay(50);
      while (live_client.available())
      {
        char c = live_client.read();

        if (c == '\n')
        {
          if (getAll.length() == 0)
          {
            state = true;
          }
          getAll = "";
        }
        else if (c != '\r')
        {
          getAll += String(c);
        }

        if (state == true)
        {
          getBody += String(c);
        }
        startTimer = millis();
      }

      if (getBody.length() > 0)
      {
        break;
      }
    }

    // Serial.println("to tera usuwom!");

    // Serial.println();

    // Serial.println(getBody);
    // Serial.println();
  }
  else
  {
    getBody = "Connection to " + serverIP + " failed.";
    // Serial.println(getBody);
  }

  return getBody;
}