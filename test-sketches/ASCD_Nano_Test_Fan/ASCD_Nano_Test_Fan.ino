// ASDC Nano 4x Arduino Charger / Discharger
// ---------------------------------------------------------------------------
// Created by Brett Watt on 19/03/2019
// Copyright 2018 - Under creative commons license 3.0:
//
// This software is furnished "as is", without technical support, and with no
// warranty, express or implied, as to its usefulness for any purpose.
//
// @brief
// ASDC Nano 4x Arduino Charger / Discharger
// Code for testing the Fan PWM speed control
// Version 1.0.0
//
// @author Email: info@vortexit.co.nz
//       Web: www.vortexit.co.nz

// Fan pin 5 PWM Digital
const byte FAN = 5; // PCB Version 1.11+ only

byte ambientTemperature = 23;

void setup()
{
  // FAN
  pinMode(FAN, OUTPUT);
  Serial.begin(115200);
  Serial.println("PWM Fan Test Code...");
  digitalWrite(FAN, LOW);
}

void loop()
{

  if (ambientTemperature > 36)
  {
    ambientTemperature = 23;
  }
  else
  {
    ambientTemperature++;
  }
  fanController();
  Serial.print("Ambient Temperature:");
  Serial.println(ambientTemperature);
  delay(2000);
}

void fanController()
{
  const byte fanTempMin = 25; // The temperature to start the fan
  const byte fanTempMax = 35; // The maximum temperature when fan is at 100%
  if (ambientTemperature < fanTempMin)
  {
    digitalWrite(FAN, LOW);
    Serial.println("FAN OFF");
  }
  else if (ambientTemperature < fanTempMax)
  {
    byte fanSpeed = map(ambientTemperature, fanTempMin, fanTempMax, 50, 200);
    Serial.print("FAN Speed:");
    Serial.println(fanSpeed);
    analogWrite(FAN, fanSpeed); // PWM speed control the FAN pin
  }
  else
  {
    digitalWrite(FAN, HIGH);
    Serial.println("FAN ON FULL");
  }
}
