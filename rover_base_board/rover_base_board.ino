#include <Servo.h>
#include <SPI.h>
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

long duration; // variable for the duration of sound wave travel
int distance;  // variable for the distance measurement

bool colideToggle = false;
bool doesForward = false;

bool toggle = false;
void (*resetFunc)(void) = 0;
// const int RECV_PIN = 12;
// IRrecv irrecv(RECV_PIN);
// decode_results results;

char buff[48];

volatile byte index;
volatile bool receivedone; /* use reception complete flag */

Servo myservo;

String message;

bool waiter = false;
bool xToggle = false;

unsigned long val;
unsigned long preMillis;

int deg = 0;

// Joystick variables
String xValue, yValue, tValue;
int xPos, yPos, tPos;
int xVal;
int yVal;

/**
 * BEGIN DEFINING FUNCTIONS
 */

void forward()
{
  if (colideToggle == false)
  {
    digitalWrite(ENA, HIGH);
    digitalWrite(ENB, HIGH);

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
    delay(330);
  }
}

void SerPls()
{
  // Serial.println("plusik");
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
  // Serial.println("minusik");
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
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(ECHO_PIN, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  // Displays the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

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

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(5200);
  digitalWrite(LED_BUILTIN, LOW);
  // irrecv.enableIRIn();
  // irrecv.blink13(true);
  myservo.attach(servopin, 560, 2000);
  myservo.write(0);

  deg = myservo.read();

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

  SPCR |= bit(SPE); /* Enable SPI */

  pinMode(MISO, OUTPUT); /* Make MISO pin as OUTPUT */
  pinMode(MOSI, INPUT);

  index = 0;
  receivedone = false;

  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
  SPI.attachInterrupt(); /* Attach SPI interrupt */

  delay(50);

  Serial.begin(19200);
  Serial.println("START");
}

//-------------------------------------------------------------------------------------------

void SPIHandler(String messageRaw)
{
  message = "";

  for (int i = 0; i <= messageRaw.length() - 2; i++)
  {
    message = message + messageRaw[i];
  }

  Serial.println(message);
  message = String(message);

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
    // Serial.println(xPos);
    // Serial.println(yPos);
    for (int i = xPos + 1; i < yPos; i++)
    {
      xValue = xValue + message[i];
    }

    for (int i = yPos + 1; i < tPos; i++)
    {
      yValue = yValue + message[i];
    }
    // Serial.print(xValue.toInt());
    // Serial.print("\t");
    // Serial.print(yValue.toInt());
    // Serial.print("\t");
    // Serial.println(tValue.toInt());
    joystickSterring(xValue.toInt(), yValue.toInt(), tValue.toInt());
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

    delay(800);

    stoper();
  }
  else if (message == "/lprec2")
  {
    left();

    delay(800);

    stoper();
  }
  else if (message == "/lprec3")
  {
    back();

    delay(800);

    stoper();
  }
  else if (message == "/lprec4")
  {
    right();

    delay(800);

    stoper();
  }
  else if (message == "/prec1")
  {
    forward();

    delay(500);

    stoper();
  }
  else if (message == "/prec2")
  {
    left();

    delay(500);

    stoper();
  }
  else if (message == "/prec3")
  {
    back();

    delay(500);

    stoper();
  }
  else if (message == "/prec4")
  {
    right();

    delay(500);

    stoper();
  }
  else if (message == "/sprec1")
  {
    forward();

    delay(150);

    stoper();
  }
  else if (message == "/sprec2")
  {
    left();

    delay(150);

    stoper();
  }
  else if (message == "/sprec3")
  {
    back();

    delay(150);

    stoper();
  }
  else if (message == "/sprec4")
  {
    right();

    delay(150);

    stoper();
  }
  else if (message == "/uprec1")
  {
    forward();

    delay(85);

    stoper();
  }
  else if (message == "/uprec2")
  {
    left();

    delay(85);

    stoper();
  }
  else if (message == "/uprec3")
  {
    back();

    delay(85);

    stoper();
  }
  else if (message == "/uprec4")
  {
    right();

    delay(85);

    stoper();
  }
  else if (message == "/0") // USTAWIENIE PINOW NA LOW ABY WYLACZYC RUCH LAZIKA
  {
    stoper();
  }

  else if (message == "/servoplus")
  {
    SerPls();
    Serial.println("servo");
  }
  else if (message == "/servominus")
  {
    SerMin();
  }
}

//-------------------------------------------------------------------------------------------

void loop()
{
  // Serial.println("co");
  delay(15);

  if (receivedone) /* Check and print received buffer if any */
  {
    buff[index] = 0;
    Serial.println(buff);
    SPIHandler(String(buff));

    index = 0;
    receivedone = false;
  }

  if (getDistance() < 30)
  {

    if ((colideToggle == false) && doesForward == true)
    {
      stoper();
    }

    colideToggle = true;
  }
  else
  {
    colideToggle = false;
  }
}

// Handler for SPI communication

ISR(SPI_STC_vect)
{
  uint8_t oldsrg = SREG;
  cli();
  char c = SPDR;
  // String jojko = "Hello\n";
  // Serial.println(jojko.toInt());
  // Serial.println(String(c));
  if (index < sizeof buff)
  {
    buff[index++] = c;
    // Serial.println(c);
    if (c == '\n')
    { /* Check for newline character as end of msg */
      receivedone = true;
    }
  }
  SREG = oldsrg;
}