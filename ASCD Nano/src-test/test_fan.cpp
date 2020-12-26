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

#include <Arduino.h>

// Fan pin 5 PWM Digital
const byte FAN = 5; // PCB Version 1.11+ only



void setup()
{
  // FAN
  pinMode(FAN, OUTPUT);
  Serial.begin(115200);
  Serial.println("PWM Fan Test Code...");
  digitalWrite(FAN, HIGH);
  delay(5000);
}

void loop()
{
  for (byte i = 100; i < 256; i++)
  {
    Serial.print("FAN Speed:");
    Serial.println(i);
    analogWrite(FAN, i); // PWM speed control the FAN pin
    delay(500);
  }
}