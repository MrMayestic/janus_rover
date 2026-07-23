/*
  The rover consists two different boards. The first is ESP32 that acts as web server.
  It provides user's control of rover and can send emails, http requests, provide camera footage and etc.
  The second board is Arduino MEGA 2560 which is mounted directly onto rover's upper plate.
  This board handles the sensors (e.g. movement and temperature sensors) and steers the rover's wheel engines.
  Communication is provided via an SPI interface.
*/

// ── Core Arduino libraries ─────────────────────────
#include <Servo.h>
#include <SPI.h>
#include "dht.h"
#include <avr/power.h>

// ── SPI pins ─────────────────────────
#define SPICLOCK 52   // sck
#define CHIPSELECT 53 // ss
// static const int spiClk = 10000000; // Clock for SPI

// ── Servo pin ─────────────────────────
#define servopin 3

// ── PIR pin ─────────────────────────
#define PIRpin 40

// ── DHT sensor ─────────────────────────
#define dht_apin 22
dht DHT;

// ── Servo state ─────────────────────────
Servo myservo;
int deg = 0;

// ── Telemetry ─────────────────────────
int currTemp = 0;
int currHumi = 0;

// ── Ultrasonic ─────────────────────────
unsigned long duration; // variable for the duration of sound wave travel
int distance;           // variable for the distance measurement

// ── Movement state flags ─────────────────────────
bool colideToggle = false;
bool doesForward = false;
bool isMoving = false;
bool xToggle = false;

// ── Global flags ─────────────────────────
bool lowEnergyMode = false;

// ── Joystick/command parsing ─────────────────────────
int xPos, yPos, tPos;
String xValue, yValue, tValue;

// ── Timings/watchdogs ─────────────────────────
unsigned long prevMillisUSS = 0;
unsigned long prevMillisSEND = 0;
unsigned long prevMillisSTOP = 0;
unsigned long prevMillisPIR = 0;
unsigned long prevMillisControl = 0;

unsigned long moveDetectionTimeout = 11000;
unsigned long timeToStop = -1;
unsigned int ultrasonicInterval = 100;

// ── SPI link ─────────────────────────
String dataRec;
uint8_t oldsrg;
uint8_t c;
volatile bool commandReady = false;

// ── Internal modules (are using extern variables from this file) ─────────────────────────
#include "src/motor/motor_control.h"
#include "src/servo/servo_control.h"
#include "src/sensors/sensors.h"
#include "src/spi/spi_comm.h"
#include "src/commands/command_handler.h"

String getReadableTime()
{
  String readableTime;

  unsigned long currentMillis;
  unsigned long seconds;
  unsigned long minutes;
  unsigned long hours;
  unsigned long days;

  currentMillis = millis();
  seconds = currentMillis / 1000;
  minutes = seconds / 60;
  hours = minutes / 60;
  days = hours / 24;
  currentMillis %= 1000;
  seconds %= 60;
  minutes %= 60;
  hours %= 24;

  readableTime += String(hours) + ":";

  if (minutes < 10)
  {
    readableTime += "0";
  }
  readableTime += String(minutes) + ":";

  if (seconds < 10)
  {
    readableTime += "0";
  }
  readableTime += String(seconds);

  return readableTime;
}

void setup()
{
  // power_adc_disable();
  power_usart1_disable();
  power_usart2_disable();
  // power_timer1_disable();
  // power_timer2_disable();
  // // power_timer3_disable();
  // power_timer4_disable();
  // power_timer5_disable();
  power_twi_disable();

  for (int i = 0; i <= 53; i++)
  {
    if (i == 12)
      continue;
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }

  pinMode(LED_BUILTIN, OUTPUT);
  // digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  // irrecv.enableIRIn();
  // irrecv.blink13(true);
  myservo.attach(servopin, 560, 2000);
  myservo.write(0);

  deg = myservo.read();

  pinMode(MOSI, INPUT);
  pinMode(MISO, OUTPUT);
  pinMode(SPICLOCK, INPUT);
  pinMode(CHIPSELECT, INPUT_PULLUP);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(ECHO_PIN, INPUT);  // Sets the echoPin as an INPUT
  pinMode(PIRpin, INPUT);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  SPI.begin();
  SPCR &= ~_BV(MSTR);
  SPCR |= _BV(SPE);

  SPI.attachInterrupt(); /* Attach SPI interrupt */

  delay(25);

  Serial.begin(230400);
  Serial.println("START");
}

/*Loop function*/

void loop()
{
  if (commandReady)
  {
    Serial.println(dataRec);
    handleIncomingRequests(dataRec);
    dataRec = "";
    commandReady = false;
  }

  if (timeToStop > 0)
  {
    if (millis() - prevMillisSTOP >= timeToStop)
    {
      stoper();
      timeToStop = -1;
    }
  }

  if (millis() - prevMillisControl >= 2200)
  {
    stoper();
    delay(500);

    prevMillisControl = millis();
  }

  if (millis() - prevMillisSEND >= 4000)
  {
    DHT.read11(dht_apin);

    prevMillisSEND = millis();

    bool changedToggle = false;

    if ((int)DHT.temperature != currTemp)
    {
      currTemp = (int)DHT.temperature;
      changedToggle = true;
    }
    if ((int)DHT.humidity != currHumi)
    {
      currHumi = (int)DHT.humidity;
      changedToggle = true;
    }
    if (changedToggle)
    {
      sendData("_t" + String(currTemp) + "h" + String(currHumi) + "v" + String(getAccurateVoltage()) + "");
    }

    // sendData("test");
  }
  if (millis() - prevMillisUSS >= ultrasonicInterval && !lowEnergyMode)
  {
    if (getDistance() < 29)
    {
      if (!lowEnergyMode)
      {
        // digitalWrite(LED_BUILTIN, HIGH);
      }

      if ((colideToggle == false) && doesForward == true)
      {
        stoper();
      }
      colideToggle = true;
      delay(500);
    }
    else
    {
      if (!lowEnergyMode)
      {
        // digitalWrite(LED_BUILTIN, LOW);
      }
      colideToggle = false;
    }
    prevMillisUSS = millis();
  }

  if (millis() - prevMillisPIR >= moveDetectionTimeout)
  {
    moveDetectionTimeout = 50;

    int result = digitalRead(PIRpin);
    if (result && !isMoving)
    {
      stoper();
      Serial.println("move");
      sendData("MOVE");
      moveDetectionTimeout = 1500;
    }
    prevMillisPIR = millis();
  }

  if (isMoving)
  {
    moveDetectionTimeout = 15000;
  }
}