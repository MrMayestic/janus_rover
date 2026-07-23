#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H
#include "Arduino.h"

#include "../motor/motor_control.h"
#include "../servo/servo_control.h"
#include "../spi/spi_comm.h"

extern int xPos, yPos, tPos;
extern String xValue, yValue, tValue;
extern unsigned long timeToStop;
extern unsigned long prevMillisSTOP;
extern unsigned long prevMillisControl;
extern int currTemp;
extern int currHumi;
extern bool lowEnergyMode;

// Forward declaration - defined in the .ino, needed here since headers aren't auto-prototyped like the .ino
String getReadableTime();

/*Function that handles messages from Master for example: to steering*/

void handleIncomingRequests(String message)
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
  else if (message == "/0")
  {
    stoper();
  }
  else if (message == "/servoplus")
  {
    SerPls();
  }
  else if (message == "/servominus")
  {
    SerMin();
  }
  else if (message == "lowEn")
  {
    lowEnergyMode = true;
    digitalWrite(LED_BUILTIN, LOW);
  }
  else if (message == "alv")
  {
    prevMillisControl = millis();
  }
}

#endif
