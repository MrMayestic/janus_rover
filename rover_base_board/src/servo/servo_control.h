#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H
#include "Arduino.h"
#include <Servo.h>

extern Servo myservo;
extern int deg;

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

#endif
