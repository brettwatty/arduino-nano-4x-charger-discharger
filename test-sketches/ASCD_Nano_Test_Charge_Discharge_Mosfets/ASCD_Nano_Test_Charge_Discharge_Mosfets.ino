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
// Code for testing the Charge and Discharge Mosfets
//
// Version 1.0.0
//
// Discharge Mosfet Test:
// Use your multimeter in continuity mode.
// One probe on GND and the other on the Drain pin of the Discharge Mosfet.
// The code cycles 1 second LOW then 1 second HIGH.
// You should hear a beep every second.
//
// Charge Mosfet Test:
// Use your multimeter in voltage mode.
// One probe on GND and the other on the +VCC pin on the TP5100 footprint.
// The code cycles 1 second LOW then 1 second HIGH.
// You should get around 12v on the +VCC pin with a 12v power adaptor plugged in.
//
// @author Email: info@vortexit.co.nz
//       Web: www.vortexit.co.nz

// Latch pin (ST_CP) of 74HC595
const byte latchPin = 7;
// Clock pin (SH_CP) of 74HC595
const byte clockPin = 8;
// Data in (DS) of 74HC595
const byte dataPin = 6;

const byte modules = 4; // Number of Modules

typedef struct
{
    // Pin Definitions
    const byte chargeMosfetPin;
    const byte dischargeMosfetPin;
} Modules;

Modules module[modules] =
    {
        {0, 1},
        {2, 3},
        {4, 5},
        {6, 7}};

void setup()
{

    // Set pins to output so you can control the shift register
    pinMode(latchPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(dataPin, OUTPUT);
    digitalWrite(latchPin, LOW);
}

void loop()
{
    for (byte i = 0; i < modules; i++)
    {
        digitalSwitch(module[i].chargeMosfetPin, 1);
        digitalSwitch(module[i].dischargeMosfetPin, 1);
    }
    delay(1000);
    for (byte i = 0; i < modules; i++)
    {
        digitalSwitch(module[i].chargeMosfetPin, 0);
        digitalSwitch(module[i].dischargeMosfetPin, 0);
    }
    delay(1000);
}

void digitalSwitch(byte j, bool value)
{
    byte baseTwo = 1;
    byte eightBitDecimal = 0;
    static bool digitalPinsState[8]; // Boolean Array for all the Shift Register Pin Values (LOW/HIGH - 0/1)
    digitalPinsState[j] = value;
    for (byte i = 0; i < 8; i++)
    {
        if (digitalPinsState[i] == 1)
            eightBitDecimal += baseTwo;
        baseTwo *= 2;
    }
    // Take the latchPin LOW
    digitalWrite(latchPin, LOW);
    // Shift out the bits:
    shiftOut(dataPin, clockPin, MSBFIRST, eightBitDecimal);
    // Take the latchPin HIGH
    digitalWrite(latchPin, HIGH);
}