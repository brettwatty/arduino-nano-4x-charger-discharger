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
// Code for getting the Mid Voltage (Between Change On / Off) for the TP5100 Chargers
// 
// 1. Remove any batteries
// 2. Upload this Sketch to the Nano
// 3. Remove the USB cable
// 4. Insert a 12V Power Supply into the DC Jack
// 5. Note the Mid Volt per module. Change it in the ASCD_Nano Main sketch in CustomSettings struct -> e.g. const float chargeLedPinMidVolatge[4] = {1.83, 1.77, 1.85, 1.85};
// 
// Version 1.0.0
//
// @author Email: info@vortexit.co.nz
//       Web: www.vortexit.co.nz

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

// Latch pin (ST_CP) of 74HC595
const byte latchPin = 7;
// Clock pin (SH_CP) of 74HC595
const byte clockPin = 8;
// Data in (DS) of 74HC595
const byte dataPin = 6;

// Mux control pins
const byte S0 = 12;
const byte S1 = 11;
const byte S2 = 10;
const byte S3 = 9;

// Mux in SIG pin Analog A0
const byte SIG = 14;

typedef struct
{
    // Pin Definitions
    const bool batteryVolatgePin[4];
    const bool batteryVolatgeDropPin[4];
    const bool chargeLedPin[4];
    const byte chargeMosfetPin;
    const byte dischargeMosfetPin;
} Modules;

Modules module[4] =
    {
        {{1, 1, 0, 1}, {1, 1, 1, 1}, {0, 1, 0, 1}, 0, 1},
        {{1, 0, 0, 1}, {0, 1, 1, 1}, {0, 0, 0, 1}, 2, 3},
        {{1, 1, 1, 0}, {1, 0, 1, 1}, {0, 1, 1, 0}, 4, 5},
        {{1, 0, 1, 0}, {0, 0, 1, 1}, {0, 0, 1, 0}, 6, 7}};

typedef struct
{
    const float voltageAREF = 1.091; // 1.1v Internal Regulator AREF measured Voltage
    const byte moduleCount = 4;      // Number of Modules
} CustomSettings;

CustomSettings settings;

float chargeLedPinMid[4];

void setup()
{
    // Set pins to output so you can control the shift register
    pinMode(latchPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(dataPin, OUTPUT);
    digitalWrite(latchPin, LOW);

    // MUX Initialization
    pinMode(S0, OUTPUT);
    pinMode(S1, OUTPUT);
    pinMode(S2, OUTPUT);
    pinMode(S3, OUTPUT);
    digitalWrite(S0, LOW);
    digitalWrite(S1, LOW);
    digitalWrite(S2, LOW);
    digitalWrite(S3, LOW);

    //Startup Screen
    lcd.init();
    lcd.clear();
    lcd.backlight(); // Turn on backlight

    for (byte i = 0; i < settings.moduleCount; i++)
    {
        digitalSwitch(module[i].chargeMosfetPin, 1);
        delay(500);
        float tempCPin = readMux(module[i].chargeLedPin);
        float chargeLedPinOff = tempCPin;
        lcd.setCursor(0, 0);
        lcd.print(i + 1);
        lcd.setCursor(1, 0);
        lcd.print(F(" CLP OFF:"));
        lcd.setCursor(10, 0);
        lcd.print(tempCPin);
        lcd.setCursor(14, 0);
        lcd.print(F("V "));
        digitalSwitch(module[i].dischargeMosfetPin, 1);
        delay(500);
        tempCPin = readMux(module[i].chargeLedPin);
        float chargeLedPinOn = tempCPin;
        lcd.setCursor(0, 1);
        lcd.print(i + 1);
        lcd.setCursor(1, 1);
        lcd.print(F(" CLP ON : "));
        lcd.setCursor(10, 1);
        lcd.print(tempCPin);
        lcd.setCursor(14, 1);
        lcd.print(F("V "));
        chargeLedPinMid[i] = (chargeLedPinOff + chargeLedPinOn) / 2;
        digitalSwitch(module[i].chargeMosfetPin, 0);
        digitalSwitch(module[i].dischargeMosfetPin, 0);
        delay(2000);
    }
    lcd.clear();
}

void loop()
{
    for (byte i = 0; i < settings.moduleCount; i++)
    {
        lcd.setCursor(0, 0);
        lcd.print(F("Module: "));
        lcd.setCursor(7, 0);
        lcd.print(i + 1);
        lcd.setCursor(8, 0);
        lcd.print(F(" MidVolt"));
        lcd.setCursor(0, 1);
        lcd.print(chargeLedPinMid[i]);
        lcd.setCursor(4, 1);
        lcd.print(F("V               "));
        delay(5000);
    }
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

float readMux(bool inputArray[])
{
    const byte controlPin[] = {S0, S1, S2, S3};

    // Loop through the 4 sig
    for (byte i = 0; i < 4; i++)
    {
        digitalWrite(controlPin[i], inputArray[i]);
    }
    // Read the value at the SIG pin 10x and convert to voltage
    float batterySampleVoltage = 0.00;
    for (byte i = 0; i < 10; i++)
    {
        if (i > 1)
            batterySampleVoltage = batterySampleVoltage + (analogRead(SIG) * readVcc() / 1024); // Dispose of the first 2x readings
    }
    batterySampleVoltage = (batterySampleVoltage / 8);
    return batterySampleVoltage; // Calculate and return the Voltage Reading
    //return batterySampleVoltage * settings.referenceVoltage / 1023.0; // Calculate and return the Voltage Reading
}

float readVcc()
{
    long result;
    // Read 1.1V reference against AVcc
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
#else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif
    delay(2);            // Wait for Vref to settle
    ADCSRA |= _BV(ADSC); // Convert
    while (bit_is_set(ADCSRA, ADSC))
        ;
    result = ADCL;
    result |= ADCH << 8;
    return ((settings.voltageAREF * 1024 * 1000) / result) / 1000; // Calculate Vcc (in mV); 1126400 = 1.1*1024*1000
}