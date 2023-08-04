/*Libraires and files including*/
#include <SPI.h>
#include <Arduino.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <string.h>

#include "esp_camera.h"
#include "esp_wifi.h"
#include "esp_bt.h"
#include "esp32-hal-cpu.h"

#include "soc/soc.h" // disable brownout problems
#include "soc/rtc_cntl_reg.h"

#include "index.h"
#include "joystick.h"

#include "wifi_config.h"

#include "SPIFFS.h"

#include "time.h"

/*Defining pins/numbers etc.*/

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

/*Start defining variables*/

String moveTimes[342];

char timeAll[24];

int moveCounter = 0;

String moveString = "";

const char *ntpServer = "vega.cbk.poznan.pl"; // Set time server
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 7200;

int currTemperature = 0;
int currHumidity = 0;

const int serverPort = server_port; // Server port

static const int spiClk = 4000000; // Clock for SPI

unsigned long lastTime = 0;
unsigned long timerDelay = 50;

unsigned long prevMillisSPI = 0;
unsigned long prevMillisLIVECAM = 0;
unsigned int SPIinterval = 50;

bool canVideo = false;
bool canLoad = false;
bool canUpload = true;

bool uploadNeeded = false;

bool connected = true;

bool recivedDone = false;

bool joystickState = false;

bool lowEnergyMode = false;

String joystickType = "";

String serverIP = server_ip;     // Server IP that handles data,photos etc.
String serverPath = server_path; // Path on server

String recData;

String s;
String req;

String since_start;

String joystickPath = "http://" + String(joystick_server_ip) + "/getJoyState";
String sendDataPath = "http://" + String(serverIP) + ":8080/sendData";

String index_html = INDEX_page;       // Load index site (HTML,CSS,JS)
String joystick_html = JOYSTICK_page; // Load joystick site (HTML,CSS,JS)

char sendBuff[32];

WiFiClient live_client;
WiFiClient client;
WiFiServer server(serverPort);

/*SPI SECTION*/

SPIClass *hspi = NULL;

uint8_t c;

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
  config.jpeg_quality = 6; // 0-63 lower number means higher quality
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);

  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

/*Handler for live transmission from camera*/
String liveS;
void liveCam(WiFiClient &client)
{

  // capture a frame
  camera_fb_t *fb = esp_camera_fb_get();

  if (!fb)
  {
    Serial.println("Frame buffer could not be acquired");
    return;
  }
  liveS = "--frame\n";
  liveS += "Content-Type: image/jpeg\n\n";
  client.print(liveS);
  client.flush();
  client.write(fb->buf, fb->len);
  client.flush();
  client.print("\n");
  // return the frame buffer back to be reused
  esp_camera_fb_return(fb);
}

/*Function that handles SPI Sending to Slave (rover main board)*/

void send_data(String stringMess)
{
  memset(sendBuff, 0, sizeof(sendBuff)); // Clearing send buffor to avoid messy chars

  stringMess.toCharArray(sendBuff, stringMess.length() + 1);

  digitalWrite(HSPI_SS, LOW);

  delayMicroseconds(5);

  for (int i = 0; i <= stringMess.length() + 1; i++)
  {
    delayMicroseconds(50);
    // Prints for developing purposes
    // Serial.print(sendBuff[i]);
    // Serial.print(" ");
    // Serial.print((uint8_t)sendBuff[i]);
    // Serial.print(" ");
    hspi->transfer((uint8_t)sendBuff[i]);
  }
  delayMicroseconds(25);
  hspi->transfer((uint8_t)4);

  digitalWrite(HSPI_SS, HIGH);

  Serial.println("sended it");
}

/*Function that reads data from Slave (called from loop();)*/

void read_data()
{
  digitalWrite(HSPI_SS, LOW);
  for (int i = 0; i <= 3; i++)
  {
    c = hspi->transfer(NULL);

    if (c < 128 && c > 31)
    {
      recData += (char)c;
    }
    // If c == 4 then it means end of message
    if (c == 4)
    {
      // Serial.print("Dane otrzymane: ");
      // Serial.println(recData);
      recivedDone = true;
    }
  }
  digitalWrite(HSPI_SS, HIGH);
}

/* HTTP request functions (for sending requests via HTTP) */

/* Codes
1 - joystick
10 - send POST req with measured data
*/

void lowEnergy()
{
  digitalWrite(33, HIGH);
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

void http_request(int whichReq)
{
  int httpResponseCode;
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    Serial.println(whichReq);
    // Your Domain name with URL path or IP address with path
    if (whichReq == 1)
    {
      http.begin(joystickPath.c_str());

      // Send HTTP GET request
      httpResponseCode = http.GET();

      if (httpResponseCode > 0)
      {
        Serial.print("HTTP joystick Response code: ");
        Serial.println(httpResponseCode);

        String payload = http.getString();

        Serial.println(payload);

        send_data(payload);
      }
    }
    else if (whichReq == 10)
    {
      http.begin(sendDataPath.c_str());

      http.addHeader("Content-Type", "text/plain");

      String sendIt = "{\"temperature\":\"" + String(currTemperature) + "\",\"humidity\":\"" + String(currHumidity) + "\",\"since_start\":\"" + since_start + "\"}";
      httpResponseCode = http.POST(sendIt);

      if (httpResponseCode > 0)
      {
        Serial.print("HTTP data Response code: ");
        Serial.println(httpResponseCode);
      }
      else
      {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
  }
  else
  {
    Serial.println("WiFi Disconnected");
  }
}

/*Function that handles http responses*/

void http_resp()
{
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
    // Serial.println("request " + req);

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
      // Serial.println("Invalid request " + req);
      return;
    }

    req = req.substring(addr_start, addr_end);
    req.trim();

    if (!lowEnergyMode)
    {
      digitalWrite(33, HIGH);
    }

    // Serial.println("Request: " + req);
    s = "";

    if (req.indexOf("joy") != -1 && req != "/joystickPage")
    {
      String joyBool = "";
      int joyStart = req.indexOf("k");

      for (int i = joyStart + 1; i <= req.length() - 1; i++)
      {
        joyBool = joyBool + req[i];
      }

      // Serial.println(joyBool);

      if (joyBool == "True")
      {
        joystickState = true;
        joystickType = "phys";
        // Serial.println("TruJes");
      }
      else if (joyBool == "TrueWEB")
      {
        joystickState = true;
        joystickType = "web";
        // Serial.println("TruJesWeb");
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
        s = "HTTP/1.1 200 OK\n";
        s += "Content-Type: text/html\n\n";
        s += index_html;
        s += "\n";

        client.print(s);

        delay(5);
      }
      else if (req == "/joystickPage")
      {
        s = "HTTP/1.1 200 OK\n";
        s += "Content-Type: text/html\n\n";
        s += joystick_html;
        s += "\n";

        client.print(s);

        delay(5);
      }
    }
    else if (req == "/data")
    {
      String sendIt = "{\"temperature\":\"" + String(currTemperature) + "\",\"humidity\":\"" + String(currHumidity) + "\",\"since_start\":\"" + since_start + "\"}";

      s = "HTTP/1.1 200 OK\n";
      s += "Access-Control-Allow-Headers: *\n";
      s += "Access-Control-Allow-Origin: *\n";
      s += "Content-Type: application/json\n\n";
      s += sendIt;
      s += "\n";

      client.print(s);
    }
    /* if request is "/" then client request the first page at root "/" -> it will return our site in index.h*/
    else if (req == "/")
    { // IFs to handle requests
      s = "HTTP/1.1 200 OK\n";
      s += "Content-Type: text/html\n\n";
      s += index_html;
      s += "\n";

      client.print(s);

      delay(5);

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
    }

    else if (req == "/video")
    {
      delay(10);

      live_client = client;

      live_client.print("HTTP/1.1 200 OK\n");
      live_client.print("Access-Control-Allow-Headers: *\n");
      live_client.print("Access-Control-Allow-Origin: *\n");
      live_client.print("Content-Type: multipart/x-mixed-replace; boundary=frame\n\n");
      live_client.flush();

      canUpload = true;
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
    else if (req == "/stop")
    { // Stream off
      client.stop();
      connected = false;
    }
    else if (req == "/start")
    { // Stream on
      connected = true;
      live_client.flush();
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
    else if (req == "/sendPhoto")
    {
      sendPhoto();
    }
    else if (req == "/lowEnergy")
    {
      lowEnergy();
    }
    else if (req == "/normalEnergy")
    {
      normalEnergy();
    }
    else if (req == "/moveResults")
    {
      // moveString = "{";

      // for (int i = 1; i <= 10; i++)
      // {
      //   String data = "";
      //   for (int j = (i - 1) * 33; j <= i * 33; j++)
      //   {
      //     data += moveTimes[j];
      //   }

      //   moveString += "\"" + String(i) + "\":\"" + data + "\"";
      //   if (i < 10)
      //   {
      //     moveString += ",";
      //   }

      //   data = "";
      // }

      // moveString += "}";

      moveString = "{\"data\":\"";

      for (int i = 0; i < 340; i++)
      {
        moveString += moveTimes[i];
      }

      moveString += "\"}";
      // Serial.println(moveString);


      s = "HTTP/1.1 200 OK\n";
      s += "Access-Control-Allow-Headers: *\n";
      s += "Access-Control-Allow-Origin: *\n";
      s += "Content-Type: application/json\n\n";
      s += moveString;
      s += "\n";

      client.print(s);

      delay(200);

      moveString = "";
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

/*Setup function*/

void setup()
{

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector

  setCpuFrequencyMhz(120);

  Serial.begin(115200);

  pinMode(33, OUTPUT); // Set LED pinMode

  hspi = new SPIClass(HSPI);

  if (!SPIFFS.begin(true))
  {
    // Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  pinMode(HSPI_SS, OUTPUT);
  delay(1);

  hspi->begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_SS);

  hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(HSPI_SS, LOW);

  wifi_config_t wifi_config = {
      .sta = {
          .listen_interval = 7,
      },
  };

  delay(450);

  send_data("/0");

  WiFi.begin(wifi_ssid, wifi_password); // Connect to WiFi with primary values

  int wifiTries = 0;

  WiFi.setSleep(false);

  // Trying to connect primary WiFi

  while (WiFi.status() != WL_CONNECTED)
  {
    if (wifiTries == 20)
    {
      break;
    }

    delay(500);
    // Serial.print(".");
    wifiTries++;
  }

  // If tries of connecting to primary WiFi are >= 20 then program tries to connect to secondary WiFi beacuse primary is probably not working

  if (wifiTries >= 20)
  {
    WiFi.begin(wifi_ssid_reserve, wifi_password_reserve);

    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      // Serial.print(".");
    }

    wifiTries = NULL;
  }

  String IP = WiFi.localIP().toString();

  index_html.replace("change_this_ip", IP);
  index_html.replace("index.html", "startPage");
  index_html.replace("joystick.html", "joystickPage");

  joystick_html.replace("change_this_ip", IP);
  joystick_html.replace("index.html", "startPage");
  joystick_html.replace("joystick.html", "joystickPage");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); // Time set
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, 1);

  // Power saving
  esp_bt_controller_disable();

  delay(10);
  configCamera();
  delay(20);
  server.begin();

  lowEnergy();
}

/*Loop function*/

void loop()
{
  delay(1);
  client = server.available();

  if (((millis() - lastTime) > timerDelay) && joystickState)
  {
    if (joystickType != "web")
    {
      http_request(1);
    }

    lastTime = millis();
  }

  if (millis() - prevMillisSPI >= 1)
  {

    read_data();

    if (recivedDone == true)
    {

      String copyOfRecData = recData; // In case of recData = "" while operating on that string

      if (copyOfRecData == "MOVE")
      {
        struct tm timeinfo;

        memset(timeAll, 0, sizeof(timeAll) / sizeof(timeAll[0]));

        if (!getLocalTime(&timeinfo))
        {
          Serial.println("Failed to obtain time");
          moveTimes[moveCounter] = String("Time null") + String("|");
          moveCounter++;
          if (moveCounter >= 340)
          {
            moveCounter = 0;
          }
        }
        else
        {
          strftime(timeAll, 18, "%m-%d %H:%M:%S", &timeinfo); // Load time
          moveTimes[moveCounter] = String(timeAll) + String("|");
          moveCounter++;
          if (moveCounter >= 340)
          {
            moveCounter = 0;
          }
        }

      }
      else
      {
        int t_start = copyOfRecData.indexOf("t");
        int h_start = copyOfRecData.indexOf("h");
        int ss_start = copyOfRecData.lastIndexOf("s");
        String temperature = copyOfRecData.substring(t_start + 1, h_start);
        String humidity = copyOfRecData.substring(h_start + 1, ss_start - 1);
        String since_start_local = copyOfRecData.substring(ss_start + 1, copyOfRecData.length());
        recData = "";
        // Serial.print(temperature);
        // Serial.print(" ");
        // Serial.print(humidity);
        // Serial.print(" ");
        // Serial.print(since_start);
        // Serial.println(" ");
        if (temperature.toInt() > 0 && temperature.toInt() < 50)
        {
          currTemperature = temperature.toInt();
        }
        if (humidity.toInt() > 0 && humidity.toInt() <= 100)
        {
          currHumidity = humidity.toInt();
        }

        since_start = since_start_local;
        if (uploadNeeded)
        {
          http_request(10);
          uploadNeeded = false;
        }
      }
      recivedDone = false;
      recData = "";
    }
    prevMillisSPI = millis();
  }

  http_resp();
  // Serial.println(connected);
  if (connected == true)
  {
    if (millis() - prevMillisLIVECAM >= 21)
    {
      liveCam(live_client);
      prevMillisLIVECAM = millis();
    }
  }
}

// Function to send photo from camera to server

String sendPhoto()
{
  String getAll;
  String getBody;

  camera_fb_t *fb = NULL;
  fb = esp_camera_fb_get(); // Getting image

  if (!fb)
  {
    Serial.println("Error while getting image");
    delay(1000);
    ESP.restart();
  }

  // Serial.println("Connecting to server: " + serverIP);

  if (live_client.connect(serverIP.c_str(), 3000))
  {

    struct tm timeinfo;

    if (!getLocalTime(&timeinfo))
    {
      Serial.println("Failed to obtain time");
    }
    strftime(timeAll, 20, "%m%d_%H%M%S", &timeinfo); // Load time

    uint32_t imageLen = fb->len;
    uint32_t totalLen = imageLen;

    Serial.println("Starting request");

    live_client.println("POST " + serverPath + " HTTP/1.1");
    live_client.println("Host: " + serverIP + ":3000");
    live_client.println("Connection: keep-alive");
    live_client.println("Content-Length: " + String(imageLen));
    live_client.println("Content-Type: application/octet-stream"); // Creating first headers in HTTP request
    live_client.println();

    Serial.println("Starting sending photo");

    uint8_t *fbBuf = fb->buf;

    size_t fbLen = fb->len;

    for (size_t n = 0; n < fbLen; n = n + 1024)
    { // Start sending image
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

    esp_camera_fb_return(fb);

    int timoutTimer = 10000;
    long startTimer = millis();

    boolean state = false;

    while ((startTimer + timoutTimer) > millis())
    {
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
  }
  else
  {
    getBody = "Connection to " + serverIP + " failed.";
  }

  return getBody;
}