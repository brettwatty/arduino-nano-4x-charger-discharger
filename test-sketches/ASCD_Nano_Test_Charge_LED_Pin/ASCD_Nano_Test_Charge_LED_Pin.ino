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

// Mux control pins
const byte S0 = 12;
const byte S1 = 11;
const byte S2 = 10;
const byte S3 = 9;

// Mux in SIG pin Analog A0
const byte SIG = 14;

const byte modules = 4; // Number of Modules
const float batteryVolatgeLeak = 0.50;

typedef struct
{
    // Pin Definitions
    const bool batteryVolatgePin[4];
    const bool batteryVolatgeDropPin[4];
    const bool chargeLedPin[4];
    const byte chargeMosfetPin;
    const byte dischargeMosfetPin;
} Modules;

Modules module[modules] =
    {
        {{1, 1, 0, 1}, {1, 1, 1, 1}, {0, 1, 0, 1}, 0, 1},
        {{1, 0, 0, 1}, {0, 1, 1, 1}, {0, 0, 0, 1}, 2, 3},
        {{1, 1, 1, 0}, {1, 0, 1, 1}, {0, 1, 1, 0}, 4, 5},
        {{1, 0, 1, 0}, {0, 0, 1, 1}, {0, 0, 1, 0}, 6, 7}};

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

    //Initialize Serial
    Serial.begin(115200);

    for (byte i = 0; i < modules; i++)
    {
        digitalSwitch(module[i].chargeMosfetPin, 1);
        digitalSwitch(module[i].dischargeMosfetPin, 0);
    }
}

void loop()
{
    float pinChargeVoltageOff = 0;
    float pinChargeVoltageOn = 0;

    Serial.println("Leave batteries out!");
    Serial.println("Taking voltage measurements....");

    for (byte i = 0; i < 10; i++)
    {
        for (byte i = 0; i < modules; i++)
        {
            float modulechargeLedPin = readMux(module[i].chargeLedPin);
            pinChargeVoltageOff += modulechargeLedPin;
            Serial.print("Module ");
            Serial.print(i + 1);
            Serial.print(" Voltage = ");
            Serial.print(modulechargeLedPin);
            Serial.println("v");
            delay(250);
        }
        delay(500);
    }
    pinChargeVoltageOff = pinChargeVoltageOff / (10 * modules);
    Serial.print("Charge Pin Off Voltage average = ");
    Serial.print(pinChargeVoltageOff);
    Serial.println("v");

    //-------------------------------------------------------------------------
    Serial.println("");
    Serial.println("Insert batteries that need charging into each slot...");
    for (byte i = 0; i < modules; i++)
    {
        digitalSwitch(module[i].chargeMosfetPin, 1);
        delay(500);
        digitalSwitch(module[i].chargeMosfetPin, 0);
        delay(500);
        readMux(module[i].batteryVolatgePin); // Read each batteryVolatgePin to pre pull down voltage to 0.00v
        digitalSwitch(module[i].chargeMosfetPin, 1);
        delay(500);
    }
    delay(5000);
    bool batteriesInserted = false;
    byte batteriesInsertedCount = 0;
    while (batteriesInserted == false)
    {
        batteriesInsertedCount = 0;
        for (byte i = 0; i < modules; i++)
        {
            if (readMux(module[i].batteryVolatgePin) >= batteryVolatgeLeak)
                batteriesInsertedCount++;
            delay(250);
        }
        if (batteriesInsertedCount >= modules)
            batteriesInserted = true;
        delay(1000);
    }
    Serial.println("Leave batteries in!");
    Serial.println("Taking voltage measurements....");

    for (byte i = 0; i < 10; i++)
    {
        for (byte i = 0; i < modules; i++)
        {
            float modulechargeLedPin = readMux(module[i].chargeLedPin);
            pinChargeVoltageOn += modulechargeLedPin;
            Serial.print("Module ");
            Serial.print(i + 1);
            Serial.print(" Voltage = ");
            Serial.print(modulechargeLedPin);
            Serial.println("v");
            delay(250);
        }
        delay(500);
    }
    pinChargeVoltageOn = pinChargeVoltageOn / (10 * modules);
    Serial.print("Charge Pin On Voltage average = ");
    Serial.print(pinChargeVoltageOn);
    Serial.println("v");
    float chargeLedPinVoltage = (pinChargeVoltageOn + pinChargeVoltageOff) / 2;
    Serial.print("Charge Pin Voltage = ");
    Serial.print(chargeLedPinVoltage);
    Serial.println("v");
    while (1)
        ;
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
    for (byte i = 0; i < 5; i++)
    {
        batterySampleVoltage = batterySampleVoltage + (analogRead(SIG) * (readVcc() / 1000.0) / 1023.0);
    }
    batterySampleVoltage = batterySampleVoltage / 10;
    return batterySampleVoltage;
}

long readVcc()
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
    result = 1126400L / result; // Calculate Vcc (in mV); 1126400 = 1.1*1024*1000
    return result;
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
