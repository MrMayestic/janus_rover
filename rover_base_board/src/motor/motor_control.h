#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H
#include "Arduino.h"

/*define channel enable output pins*/
#define ENA 5 // Left  wheel speed
#define ENB 6 // Right wheel speed
/*define logic control output pins*/
#define IN1 7        // Left  wheel forward
#define IN2 8        // Left  wheel reverse
#define IN3 9        // Right wheel reverse
#define IN4 11       // Right wheel forward
#define carSpeed 250 // initial speed of car >=0 to <=255

extern bool colideToggle;
extern bool doesForward;
extern bool isMoving;
extern bool waiter;
extern bool xToggle;

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
    isMoving = true;
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
  isMoving = true;
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
  isMoving = true;
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
  isMoving = true;
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
  isMoving = false;
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

#endif
