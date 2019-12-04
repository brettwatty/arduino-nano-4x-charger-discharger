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

byte ambientTemperature = 20;

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

  if (ambientTemperature > 40)
  {
    ambientTemperature = 20;
  }
  else
  {
    ambientTemperature++;
  }
  fanController();
  Serial.print("Ambient Temperature:");
  Serial.println(ambientTemperature);
  delay(10000);
}

void fanController()
{
  static boolean fanOn = 0;
  const byte fanTempMin = 25; // The temperature to start the fan
  const byte fanTempMax = 35; // The maximum temperature when fan is at 100%
  byte fanSpeed;
  if (ambientTemperature < fanTempMin)
  {
    digitalWrite(FAN, LOW);
    Serial.println("FAN OFF");
    fanOn = 0;
  }
  else if (ambientTemperature < fanTempMax)
  {
    if (fanOn == 0) {
      fanSpeed = 255;
      fanOn = 1;
    } else {
    fanSpeed = map(ambientTemperature, fanTempMin, fanTempMax, 100, 250);
    }
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
