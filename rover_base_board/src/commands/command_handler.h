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

void handleJoystickCommand(const String &message)
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

void handleSendDataCommand()
{
  sendData("_t" + String(currTemp) + "h" + String(currHumi) + "ss" + getReadableTime() + "");
}

void handleLongPrecisionForward()
{
  forward();
  timeToStop = 800;
  prevMillisSTOP = millis();
}

void handleLongPrecisionLeft()
{
  left();
  timeToStop = 800;
  prevMillisSTOP = millis();
}

void handleLongPrecisionBack()
{
  back();
  timeToStop = 800;
  prevMillisSTOP = millis();
}

void handleLongPrecisionRight()
{
  right();
  timeToStop = 800;
  prevMillisSTOP = millis();
}

void handlePrecisionForward()
{
  forward();
  timeToStop = 500;
  prevMillisSTOP = millis();
}

void handlePrecisionLeft()
{
  left();
  timeToStop = 500;
  prevMillisSTOP = millis();
}

void handlePrecisionBack()
{
  back();
  timeToStop = 500;
  prevMillisSTOP = millis();
}

void handlePrecisionRight()
{
  right();
  timeToStop = 500;
  prevMillisSTOP = millis();
}

void handleShortPrecisionForward()
{
  forward();
  timeToStop = 150;
  prevMillisSTOP = millis();
}

void handleShortPrecisionLeft()
{
  left();
  timeToStop = 150;
  prevMillisSTOP = millis();
}

void handleShortPrecisionBack()
{
  back();
  timeToStop = 150;
  prevMillisSTOP = millis();
}

void handleShortPrecisionRight()
{
  right();
  timeToStop = 150;
  prevMillisSTOP = millis();
}

void handleUltraPrecisionForward()
{
  forward();
  timeToStop = 85;
  prevMillisSTOP = millis();
}

void handleUltraPrecisionLeft()
{
  left();
  timeToStop = 85;
  prevMillisSTOP = millis();
}

void handleUltraPrecisionBack()
{
  back();
  timeToStop = 85;
  prevMillisSTOP = millis();
}

void handleUltraPrecisionRight()
{
  right();
  timeToStop = 85;
  prevMillisSTOP = millis();
}

void handleLowEnergyCommand()
{
  lowEnergyMode = true;
  digitalWrite(LED_BUILTIN, LOW);
}

void handleKeepAliveCommand()
{
  prevMillisControl = millis();
}

/*Function that handles messages from Master for example: to steering*/

void handleIncomingRequests(String message)
{
  if (message.indexOf("x") != -1)
  {
    handleJoystickCommand(message);
  }
  else if (message == "sendData")
  {
    handleSendDataCommand();
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
    handleLongPrecisionForward();
  }
  else if (message == "/lprec2")
  {
    handleLongPrecisionLeft();
  }
  else if (message == "/lprec3")
  {
    handleLongPrecisionBack();
  }
  else if (message == "/lprec4")
  {
    handleLongPrecisionRight();
  }
  else if (message == "/prec1")
  {
    handlePrecisionForward();
  }
  else if (message == "/prec2")
  {
    handlePrecisionLeft();
  }
  else if (message == "/prec3")
  {
    handlePrecisionBack();
  }
  else if (message == "/prec4")
  {
    handlePrecisionRight();
  }
  else if (message == "/sprec1")
  {
    handleShortPrecisionForward();
  }
  else if (message == "/sprec2")
  {
    handleShortPrecisionLeft();
  }
  else if (message == "/sprec3")
  {
    handleShortPrecisionBack();
  }
  else if (message == "/sprec4")
  {
    handleShortPrecisionRight();
  }
  else if (message == "/uprec1")
  {
    handleUltraPrecisionForward();
  }
  else if (message == "/uprec2")
  {
    handleUltraPrecisionLeft();
  }
  else if (message == "/uprec3")
  {
    handleUltraPrecisionBack();
  }
  else if (message == "/uprec4")
  {
    handleUltraPrecisionRight();
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
    handleLowEnergyCommand();
  }
  else if (message == "alv")
  {
    handleKeepAliveCommand();
  }
}

#endif
