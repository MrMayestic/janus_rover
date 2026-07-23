#ifndef SENSORS_H
#define SENSORS_H
#include "Arduino.h"

#define ECHO_PIN A4
#define TRIG_PIN A5

extern unsigned long duration;
extern int distance;

/*ULTRASONIC*/

unsigned int getDistance(void)
{ // Getting distance
  digitalWrite(TRIG_PIN, LOW);

  delayMicroseconds(1);

  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(TRIG_PIN, HIGH);

  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds.
  // Timeout capped at 30ms (real max sensor range ~400cm is ~23.5ms echo) instead of
  // the 1s default, so a missing echo can't stall loop() for a full second.
  duration = pulseIn(ECHO_PIN, HIGH, 30000UL);

  if (duration == 0)
  {
    // No echo within timeout - nothing in range, not "an obstacle right at the sensor"
    distance = 9999;
  }
  else
  {
    // Calculating the distance
    distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  }
  // Displays the distance on the Serial Monitor
  // Serial.print("Distance: ");
  // Serial.print(distance);
  // Serial.println(" cm");

  return distance;
}

// Internal power voltage control

int getVoltage(void); // forward declaration - defined below, needed here since headers aren't auto-prototyped like the .ino

int getAccurateVoltage()
{
  getVoltage();
  return getVoltage();
}

// Read the voltage of the battery the Arduino is currently running on (in millivolts)
int getVoltage(void)
{
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) // For mega boards
  const long InternalReferenceVoltage = 1115L;                 // Adjust this value to your boards specific internal BG voltage x1000
  ADMUX = (0 << REFS1) | (1 << REFS0) | (0 << ADLAR) | (0 << MUX5) | (1 << MUX4) | (1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (0 << MUX0);
#else // For 168/328 boards
  const long InternalReferenceVoltage = 1091L; // Adjust this value to your boards specific internal BG voltage x1000
  ADMUX = (0 << REFS1) | (1 << REFS0) | (0 << ADLAR) | (1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (0 << MUX0);
#endif
  ADCSRA |= _BV(ADSC); // Start a conversion
  while (((ADCSRA & (1 << ADSC)) != 0))
    ;                                                                    // Wait for it to complete
  int results = (((InternalReferenceVoltage * 1024L) / ADC) + 5L) / 10L; // Scale the value; calculates for straight line value
  return results * 10;                                                   // convert from centivolts to millivolts
}

#endif
