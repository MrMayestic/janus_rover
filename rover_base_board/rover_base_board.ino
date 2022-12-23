#include <Servo.h>
#include <SPI.h>

#include "dht.h"
#define dht_apin 22
dht DHT;

#define DATAOUT 51    // MOSI
#define DATAIN 50     // MISO
#define SPICLOCK 52   // sck
#define CHIPSELECT 53 // ss

/*define channel enable output pins*/
#define ENA 5 // Left  wheel speed
#define ENB 6 // Right wheel speed
/*define logic control output pins*/
#define IN1 7        // Left  wheel forward
#define IN2 8        // Left  wheel reverse
#define IN3 9        // Right wheel reverse
#define IN4 11       // Right wheel forward
#define carSpeed 250 // initial speed of car >=0 to <=255
#define LowCarSpeed 100

#define ECHO_PIN A4
#define TRIG_PIN A5

#define servopin 3

int deg = 0;

int xPos, yPos, tPos;

// static const int spiClk = 10000000; // Clock for SPI

unsigned long prevMillisUSS = 0;
unsigned long prevMillisSEND = 0;
unsigned long prevMillisSTOP = 0;

unsigned long timeToStop = -1;

unsigned int ultrasonicInterval = 75;

int currTemp = 0;
int currHumi = 0;

unsigned long duration; // variable for the duration of sound wave travel
int distance;           // variable for the distance measurement

bool colideToggle = false;
bool doesForward = false;

bool waiter = false;
bool xToggle = false;

volatile bool receivedone; /* use reception complete flag */

String data;
String dataRec;

String xValue, yValue, tValue;

char buffer[32];

byte clr;
volatile byte index;

uint8_t oldsrg;
uint8_t c;
uint8_t recived;

void (*resetFunc)(void) = 0;

Servo myservo;

// const int RECV_PIN = 12;
// IRrecv irrecv(RECV_PIN);

/* BEGIN DEFINING FUNCTIONS */

void forward()
{
  if (colideToggle == false)
  {
    digitalWrite(ENA, HIGH);
    digitalWrite(ENB, HIGH);

    // analogWrite(ENA, 50);
    // analogWrite(ENB, 50);

    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    //  Serial.println("go forward!");
    waiter = true;
    doesForward = true;
  }
}

void back()
{
  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  //  Serial.println("go back!");
  waiter = true;
  doesForward = false;
}

void left()
{
  analogWrite(ENA, carSpeed);
  analogWrite(ENB, carSpeed);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  //  Serial.println("go left!");
  waiter = true;
  doesForward = false;
}

void right()
{
  analogWrite(ENA, carSpeed);
  analogWrite(ENB, carSpeed);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  // Serial.println("go right!");
  waiter = true;
  doesForward = false;
}

void stoper()
{
  digitalWrite(ENA, LOW);
  digitalWrite(ENB, LOW);
  doesForward = false;
  //  Serial.println("STOP!");
  if (waiter == true)
  {
    waiter = false;
    delay(150);
  }
}

void SerPls()
{
  deg = myservo.read() + 45;

  if (deg < 175)
  {
    myservo.write(deg);
  }
  else
  {
    deg = 170;
    myservo.write(deg);
  }
}

void SerMin()
{
  deg = myservo.read() - 45;

  if (deg > 0)
  {
    myservo.write(deg);
  }
  else
  {
    deg = 5;
    myservo.write(deg);
  }
}

/*ULTRASONIC*/

unsigned int getDistance(void)
{ // Getting distance
  digitalWrite(TRIG_PIN, LOW);

  delayMicroseconds(2);

  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(TRIG_PIN, HIGH);

  delay(3);

  digitalWrite(TRIG_PIN, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(ECHO_PIN, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  // Displays the distance on the Serial Monitor
  // Serial.print("Distance: ");
  // Serial.print(distance);
  // Serial.println(" cm");

  return distance;
}

// Function for joystick steering. X,Y are coordinates and mode is to tell program how rover's engines should be set in order to move along the appropriate axis.

void joystickSterring(int x, int y, int mode)
{
  if (mode == 4)
  {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);

    xToggle = false;
  }
  else if (mode == 3)
  {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);

    xToggle = false;
  }
  else if (mode == 2)
  {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);

    xToggle = true;
  }
  else if (mode == 1)
  {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);

    xToggle = true;
  }

  x = constrain(x, 0, 255);
  y = constrain(y, 0, 255);

  if (xToggle)
  {
    analogWrite(ENB, x);
    analogWrite(ENA, y);
  }
  else
  {
    analogWrite(ENA, x);
    analogWrite(ENB, y);
  }
}

// Function that handles joystick from web (wrote via js) which has diffrent working methods

void WEBjoystickSterring(int x, int y)
{
  // Serial.println(x);
  // Serial.println(y);
  if (x > 0 && colideToggle)
  {
    return;
  }

  if (y < 0)
  {

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }
  else
  {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }

  x = abs(x);
  y = abs(y);

  x = constrain(x, 0, 255);
  y = constrain(y, 0, 255);

  analogWrite(ENA, x);
  analogWrite(ENB, y);
}

void fill_buffer(String data)
{
  for (int i = 0; i <= data.length(); i++)
  {
    buffer[i] = (uint8_t)data[i];
  }
}

void sendData(String data)
{
  fill_buffer(data);

  SPI.transfer(buffer, data.length() + 1);
  // delayMicroseconds(5);
  SPI.transfer((uint8_t)4);
  // SPI.endTransaction();
}

/*Function that handles messages from Master for example: to steering*/

void requestsHandler(String message)
{
  if (message.indexOf("x") != -1)
  {
    xPos = 0;
    yPos = 0;

    xValue = "";
    yValue = "";
    tValue = "";

    xPos = message.indexOf("x");
    yPos = message.indexOf("y");
    tPos = message.indexOf("t");

    tValue = message[tPos + 1];

    for (int i = xPos + 1; i < yPos; i++)
    {
      xValue = xValue + message[i];
    }

    if (tPos > 0)
    {

      for (int i = yPos + 1; i < tPos; i++)
      {
        yValue = yValue + message[i];
      }
    }
    else
    {
      for (int i = yPos + 1; i < message.length(); i++)
      {
        yValue = yValue + message[i];
      }
    }

    // Serial.println(tValue.toInt());

    if (tValue.toInt() > 0)
    {
      joystickSterring(xValue.toInt(), yValue.toInt(), tValue.toInt());
    }
    else
    {
      WEBjoystickSterring(xValue.toInt(), yValue.toInt());
    }
  }
  else if (message == "sendData")
  {
    // Serial.println("aha");
    memset(buffer, 0, sizeof(buffer));
    sendData("_t" + String(currTemp) + "h" + String(currHumi) + "ss" + getReadableTime() + "");
  }
  else if (message == "/1")
  {
    forward();
  }
  else if (message == "/2")
  {
    left();
  }
  else if (message == "/3")
  {
    back();
  }
  else if (message == "/4")
  {
    right();
  }
  else if (message == "/lprec1")
  {

    forward();

    timeToStop = 800;

    prevMillisSTOP = millis();
  }
  else if (message == "/lprec2")
  {
    left();

    timeToStop = 800;

    prevMillisSTOP = millis();
  }
  else if (message == "/lprec3")
  {
    back();

    timeToStop = 800;

    prevMillisSTOP = millis();
  }
  else if (message == "/lprec4")
  {
    right();

    timeToStop = 800;

    prevMillisSTOP = millis();
  }
  else if (message == "/prec1")
  {
    forward();

    timeToStop = 500;

    prevMillisSTOP = millis();
  }
  else if (message == "/prec2")
  {
    left();

    timeToStop = 500;

    prevMillisSTOP = millis();
  }
  else if (message == "/prec3")
  {
    back();

    timeToStop = 500;

    prevMillisSTOP = millis();
  }
  else if (message == "/prec4")
  {
    right();

    timeToStop = 500;

    prevMillisSTOP = millis();
  }
  else if (message == "/sprec1")
  {
    forward();

    timeToStop = 150;

    prevMillisSTOP = millis();
  }
  else if (message == "/sprec2")
  {
    left();

    timeToStop = 150;

    prevMillisSTOP = millis();
  }
  else if (message == "/sprec3")
  {
    back();

    timeToStop = 150;

    prevMillisSTOP = millis();
  }
  else if (message == "/sprec4")
  {
    right();

    timeToStop = 150;

    prevMillisSTOP = millis();
  }
  else if (message == "/uprec1")
  {
    forward();

    timeToStop = 85;

    prevMillisSTOP = millis();
  }
  else if (message == "/uprec2")
  {
    left();

    timeToStop = 85;

    prevMillisSTOP = millis();
  }
  else if (message == "/uprec3")
  {
    back();

    timeToStop = 85;

    prevMillisSTOP = millis();
  }
  else if (message == "/uprec4")
  {
    right();

    timeToStop = 85;

    prevMillisSTOP = millis();
  }
  else if (message == "/0") // USTAWIENIE PINOW NA LOW ABY WYLACZYC RUCH LAZIKA
  {
    stoper();
  }

  else if (message == "/servoplus")
  {
    SerPls();
    // Serial.println("servo");
  }
  else if (message == "/servominus")
  {
    SerMin();
  }
}

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
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  // irrecv.enableIRIn();
  // irrecv.blink13(true);
  myservo.attach(servopin, 560, 2000);
  myservo.write(0);

  deg = myservo.read();

  pinMode(MISO, INPUT);
  pinMode(MISO, OUTPUT);
  pinMode(SPICLOCK, INPUT);
  pinMode(CHIPSELECT, INPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(ECHO_PIN, INPUT);  // Sets the echoPin as an INPUT

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  delay(100);

  SPCR |= _BV(SPE);

  // SPI.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  SPI.attachInterrupt(); /* Attach SPI interrupt */
  index = 0;
  receivedone = false;

  delay(50);

  Serial.begin(230400);
  Serial.println("START");
}

/*Loop function*/

void loop()
{
  // Serial.println("test");
  if (timeToStop > 0)
  {
    if (millis() - prevMillisSTOP >= timeToStop)
    {
      stoper();
      timeToStop = -1;
    }
  }

  if (millis() - prevMillisSEND >= 500)
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
      // Serial.println(currTemp);
      // Serial.println(currHumi);
      memset(buffer, 0, sizeof(buffer));
      sendData("_t" + String(currTemp) + "h" + String(currHumi) + "ss" + getReadableTime() + "");
    }

    // sendData("test");
  }
  if (millis() - prevMillisUSS >= ultrasonicInterval)
  {

    if (getDistance() < 29)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      if ((colideToggle == false) && doesForward == true)
      {
        stoper();
      }
      colideToggle = true;
      delay(500);
    }
    else
    {
      digitalWrite(LED_BUILTIN, LOW);
      colideToggle = false;
    }
    prevMillisUSS = millis();
  }
}

/* ISR that handles reciving data via SPI from Master */

ISR(SPI_STC_vect)
{
  // SPI.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  oldsrg = SREG;

  cli();

  c = SPDR;

  // Serial.println(c);
  if (c < 128 && c > 31)
  {
    dataRec += (char)c;
    // Serial.println(dataRec);
  }
  if (c == 4)
  {
    // Serial.print("Dane otrzymane u slavea: ");
    // Serial.println(dataRec);
    // Serial.println();
    if (dataRec.length() > 0)
    {
      requestsHandler(dataRec);
    }
    index = 0;
    dataRec = "";
  }
  SREG = oldsrg;
  // SPI.endTransaction();
}