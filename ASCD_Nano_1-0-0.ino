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
// Main code for the Arduino Nano 3.0 ATmega328P
// Version 1.0.0
//
// @author Email: info@vortexit.co.nz
//       Web: www.vortexit.co.nz

#define TEMPERATURE_PRECISION 9
#define ONE_WIRE_BUS 4 // Pin 4 Temperature Sensors

//Include Libraries
#include <Wire.h> // Comes with Arduino IDE
#include <OneWire.h>
#include <LiquidCrystal_I2C.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>

//Objects
LiquidCrystal_I2C lcd(0x27, 16, 2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
OneWire oneWire(ONE_WIRE_BUS);		 // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature.
SoftwareSerial ESP8266(3, 2);		 // RX , TX

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

// Button pin Analog A1
const byte BTN = 15;

// Buzzer pin Analog A2
//const byte BUZZ = 5; // PCB Version 1.1
const byte BUZZ = 16; // PCB Version 1.11

// Fan pin 5 PWM Digital
const byte FAN = 5; // PCB Version 1.11+ only

typedef struct
{
	// Pin Definitions
	const bool batteryVolatgePin[4];
	const bool batteryVolatgeDropPin[4];
	const bool chargeLedPin[4];
	const byte chargeMosfetPin;
	const byte dischargeMosfetPin;

	// Timmer
	unsigned long longMilliSecondsCleared;
	byte seconds;
	byte minutes;
	byte hours;

	// Module Cycle
	byte cycleCount;
	bool batteryBarcode;
	bool insertData;
	byte cycleState;
	byte batteryFaultCode;

	// Voltage Readings
	float batteryInitialVoltage;
	float batteryVoltage;

	// Temp Readings an Variables
	byte batteryInitialTemp;
	byte batteryHighestTemp;
	byte batteryCurrentTemp;
	byte tempCount;

	// Milli Ohms
	float tempMilliOhmsValue;
	float milliOhmsValue;

	// Discharge Battery
	int intMilliSecondsCount;
	unsigned long longMilliSecondsPreviousCount;
	unsigned long longMilliSecondsPrevious;
	unsigned long longMilliSecondsPassed;
	float dischargeMilliamps;
	float dischargeVoltage;
	float dischargeAmps;
	bool dischargeCompleted;
	int dischargeMinutes;
	bool pendingDischargeRecord;
} Modules;

Modules module[4] =
	{
		{{1, 1, 0, 1}, {1, 1, 1, 1}, {0, 1, 0, 1}, 0, 1},
		{{1, 0, 0, 1}, {0, 1, 1, 1}, {0, 0, 0, 1}, 2, 3},
		{{1, 1, 1, 0}, {1, 0, 1, 1}, {0, 1, 1, 0}, 4, 5},
		{{1, 0, 1, 0}, {0, 0, 1, 1}, {0, 0, 1, 0}, 6, 7}};

typedef struct
{
	const float shuntResistor[4] = {3.3, 3.3, 3.3, 3.3};
	const float referenceVoltage = 5.01;		   // 5V Output of Arduino
	const float defaultBatteryCutOffVoltage = 2.8; // Voltage that the discharge stops
	const byte restTimeMinutes = 1;				   // The time in Minutes to rest the battery after charge. 0-59 are valid
	const int lowMilliamps = 1000;				   //  This is the value of Milli Amps that is considered low and does not get recharged because it is considered faulty
	const int highMilliOhms = 500;				   //  This is the value of Milli Ohms that is considered high and the battery is considered faulty
	const int offsetMilliOhms = 0;				   // Offset calibration for MilliOhms
	const byte chargingTimeout = 8;				   // The timeout in Hours for charging
	const byte tempThreshold = 7;				   // Warning Threshold in degrees above initial Temperature
	const byte tempMaxThreshold = 20;			   //Maximum Threshold in degrees above initial Temperature - Considered Faulty
	const float batteryVolatgeLeak = 0.50;		   // On the initial screen "BATTERY CHECK" observe the highest voltage of each module and set this value slightly higher
	const byte moduleCount = 4;					   // Number of Modules
	const byte screenTime = 4;					   // Time in Seconds (Cycles) per Active Screen
	const int dischargeReadInterval = 5000;		   // Time intervals between Discharge readings. Adjust for mAh +/-
} CustomSettings;

CustomSettings settings;

// You need to run the Dallas get temp sensors sketch (ASCD_Test_Get_DS18B20_Serials.ino) to get your DS serails
DeviceAddress tempSensorSerial[5] = {
	{0x28, 0x81, 0xCB, 0x79, 0x97, 0x05, 0x03, 0xE5},
	{0x28, 0xAA, 0xF5, 0x6F, 0x1D, 0x13, 0x02, 0xAC},
	{0x28, 0xAA, 0x37, 0x78, 0x1D, 0x13, 0x02, 0xB3},
	{0x28, 0xAA, 0xB7, 0x71, 0x1D, 0x13, 0x02, 0xEB},
	{0x28, 0xAA, 0x2D, 0x6F, 0x1D, 0x13, 0x02, 0xCC}};

byte ambientTemperature = 0;
boolean buttonPressed = false;
boolean readSerialResponse = false;
char serialSendString[512];
byte countSerialSend = 0;
boolean soundBuzzer = false;

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

	// Button
	pinMode(BTN, INPUT);

	// Buzzer
	pinMode(BUZZ, OUTPUT);
	digitalWrite(BUZZ, HIGH);
	delay(50);
	digitalWrite(BUZZ, LOW);

	// FAN
	pinMode(FAN, OUTPUT);

	//Initialize Serial
	Serial.begin(115200);
	Serial.setTimeout(5);

	//Initialize Software Serial for communication with the ESP8266
	ESP8266.begin(57600);
	ESP8266.setTimeout(5);

	//Startup Screen
	lcd.init();
	lcd.clear();
	lcd.backlight(); // Turn on backlight
	lcd.setCursor(0, 0);
	lcd.print(F("ASCD NANO V1.0.0"));
	lcd.setCursor(0, 1);
	lcd.print(F("Init TP5100....."));

	// Set All Digital Outputs on the Shift Register to LOW
	for (byte i = 0; i < settings.moduleCount; i++)
	{
		digitalSwitch(module[i].chargeMosfetPin, 1);
		delay(500);
		digitalSwitch(module[i].chargeMosfetPin, 0);
		delay(500);
		readMux(module[i].batteryVolatgePin); // Read each batteryVolatgePin to pre pull down voltage to 0.00v
		digitalSwitch(module[i].dischargeMosfetPin, 1);
		delay(500);
		digitalSwitch(module[i].dischargeMosfetPin, 0);
		delay(500);
	}
	lcd.setCursor(0, 1);
	lcd.print(F("Starting........"));

	sensors.begin(); // Start up the library Dallas Temperature IC Control

	lcd.clear();
}

void loop()
{
	if (readSerialResponse == true)
		readSerial();

	// Mills - Timmer
	static long buttonMillis;
	static long cycleStateValuesMillis;
	static long sendSerialMillis;
	static long buzzerMillis;
	long currentMillis = millis();
	if (currentMillis - buttonMillis >= 2) // Every 2 millis
	{
		button();
		buttonMillis = currentMillis;
	}
	currentMillis = millis();
	if (currentMillis - buzzerMillis >= 50) // Every 100 millis
	{
		buzzer();
		buzzerMillis = currentMillis;
	}
	currentMillis = millis();
	if (currentMillis - cycleStateValuesMillis >= 1000) // Every 1 second
	{
		cycleStateValues();
		cycleStateValuesMillis = currentMillis;
	}
	currentMillis = millis();
	if (currentMillis - sendSerialMillis >= 4000) // Every 4 seconds
	{
		if (readSerialResponse == false || countSerialSend > 5)
		{
			sendSerial();
			countSerialSend = 0;
		}
		else
		{
			countSerialSend++;
		}
		sendSerialMillis = currentMillis;
	}
}

void buzzer()
{
	if (soundBuzzer == true)
	{
		digitalWrite(BUZZ, HIGH);
		soundBuzzer = false;
	}
	else
	{
		digitalWrite(BUZZ, LOW);
	}
}

void fanController()
{
	const byte fanTempMin = 25; // The temperature to start the fan
	const byte fanTempMax = 35; // The maximum temperature when fan is at 100%
	if (ambientTemperature < fanTempMin)
	{
		digitalWrite(FAN, LOW);
	}
	else if (ambientTemperature < fanTempMax)
	{
		byte fanSpeed = map(ambientTemperature, fanTempMin, fanTempMax, 41, 100);
		analogWrite(FAN, fanSpeed); // PWM speed control the FAN pin
	}
	else
	{
		digitalWrite(FAN, HIGH);
	}
}

void sendSerial()
{
	if (strcmp(serialSendString, "") != 0)
	{
		ESP8266.println(serialSendString);
		Serial.println(serialSendString);
		strcpy(serialSendString, "");
		readSerialResponse = true;
	}
}

void readSerial()
{
	while (ESP8266.available())
	{
		String returnString = "";
		String recievedMessage = "";
		int returnInt;
		recievedMessage = ESP8266.readString(); // Read the incoming data as string
		Serial.println(recievedMessage);
		recievedMessage.trim();
		if (recievedMessage.length() > 1) // Multiple CHAR
		{
			for (byte i = 0; i <= recievedMessage.length(); i++)
			{
				if (recievedMessage.charAt(i) == ':' || recievedMessage.length() == i)
				{
					returnInt = returnString.toInt();
					if (returnInt != 0)
					{
						returnCodes(returnInt);
					}
					else
					{
						returnCodes(9); // ERROR_SERIAL_OUTPUT
					}
					returnString = "";
				}
				else
				{
					returnString += recievedMessage.charAt(i);
				}
			}
		}
		else
		{ // Single CHAR
			returnInt = recievedMessage.toInt();
			returnCodes(returnInt);
		}
		countSerialSend = 0;
		readSerialResponse = false;
	}
}

void button()
{
	boolean buttonState = 0;
	static boolean lastButtonState = 0;
	buttonState = digitalRead(BTN);
	if (buttonState != lastButtonState)
	{
		if (buttonState == LOW)
		{
			buttonPressed = true;
			soundBuzzer = true;
		}
	}
	lastButtonState = buttonState;
}

void cycleStateLCD()
{
	static byte screenOverride = 0;
	static byte cycleStateCount;
	static byte cycleStateActive = 0; // The Active Module on the LCD

	if (screenOverride > 0 && buttonPressed == false)
	{
		screenOverride--;
		cycleStateLCDOutput(cycleStateActive);
	}
	else if (screenOverride == 0 && buttonPressed == true)
	{
		lcd.setCursor(0, 0);
		lcd.print(F("LOCK MODE 1 MIN "));
		lcd.setCursor(0, 1);
		lcd.print(F("                "));
		screenOverride = 60;
		buttonPressed = false;
	}
	else
	{
		if (cycleStateCount == settings.screenTime || buttonPressed == true) // Screen Time in seconds defined in CustomSettings
		{
			if (cycleStateActive == (settings.moduleCount - 1)) // 0-3 is 4x Modules
			{
				cycleStateActive = 0;
			}
			else
			{
				cycleStateActive++;
			}
			cycleStateCount = 0;
			buttonPressed = false;
		}
		else
		{
			cycleStateCount++;
		}
		cycleStateLCDOutput(cycleStateActive);
	}
}

void secondsTimer(byte j)
{
	unsigned long longMilliSecondsCount = millis() - module[j].longMilliSecondsCleared;
	module[j].hours = longMilliSecondsCount / (1000L * 60L * 60L);
	module[j].minutes = (longMilliSecondsCount % (1000L * 60L * 60L)) / (1000L * 60L);
	module[j].seconds = (longMilliSecondsCount % (1000L * 60L * 60L) % (1000L * 60L)) / 1000;
}

void clearSecondsTimer(byte j)
{
	module[j].longMilliSecondsCleared = millis();
	module[j].seconds = 0;
	module[j].minutes = 0;
	module[j].hours = 0;
}

void initializeVariables(byte j)
{
	// Initialization
	module[j].batteryBarcode = false;
	module[j].insertData = false;
	module[j].tempMilliOhmsValue = 0;
	module[j].milliOhmsValue = 0;
	module[j].intMilliSecondsCount = 0;
	module[j].longMilliSecondsPreviousCount = 0;
	module[j].longMilliSecondsPrevious = 0;
	module[j].longMilliSecondsPassed = 0;
	module[j].dischargeMilliamps = 0.0;
	module[j].dischargeVoltage = 0.00;
	module[j].dischargeAmps = 0.00;
	module[j].dischargeCompleted = false;
	module[j].batteryFaultCode = 0;
	module[j].batteryInitialTemp = 0;
	module[j].batteryCurrentTemp = 0;
	module[j].batteryHighestTemp = 0;
}

void cycleStateValues()
{
	strcpy(serialSendString, "");
	getAmbientTemperature();
	//Serial.println(ambientTemperature);
	sprintf_P(serialSendString + strlen(serialSendString), PSTR("&AT=%d"), ambientTemperature);
	for (byte i = 0; i < settings.moduleCount; i++)
	{
		switch (module[i].cycleState)
		{
		case 0: // Check Battery Voltage
			if (batteryCheck(i))
				module[i].cycleCount++;
			if (module[i].cycleCount == 5)
			{
				initializeVariables(i);
				module[i].batteryCurrentTemp = getTemperature(i);
				module[i].batteryInitialTemp = module[i].batteryCurrentTemp;
				module[i].batteryHighestTemp = module[i].batteryCurrentTemp;
				clearSecondsTimer(i);
				module[i].batteryVoltage = readMux(module[i].batteryVolatgePin); // Get battery voltage for Charge Cycle
				module[i].batteryInitialVoltage = module[i].batteryVoltage;
				module[i].cycleState = 1; // Check Battery Voltage Completed set cycleState to Get Battery Barcode
				module[i].cycleCount = 0; // Reset cycleCount for use in other Cycles
			}
			sprintf_P(serialSendString + strlen(serialSendString), PSTR("&CS%d=0"), i);
			break;
		case 1:																 // Battery Barcode
			module[i].batteryVoltage = readMux(module[i].batteryVolatgePin); // Get battery voltage
			if (module[i].batteryBarcode == true)
			{
				clearSecondsTimer(i);
				module[i].batteryInitialVoltage = module[i].batteryVoltage; // Reset Initial voltage
				module[i].cycleState = 2;									// Get Battery Barcode Completed set cycleState to Charge Battery
			}
			//Check if battery has been removed
			if (!batteryCheck(i))
				module[i].cycleCount++;
			if (module[i].cycleCount == 5)
			{
				module[i].cycleState = 0; // Completed and Battery Removed set cycleState to Check Battery Voltage
				module[i].cycleCount = 0; // Reset cycleCount for use in other Cycles
			}
			sprintf_P(serialSendString + strlen(serialSendString), PSTR("&CS%d=1"), i);
			break;
		case 2: // Charge Battery
			//Serial.println(readMux(module[i].chargeLedPin));
			module[i].batteryVoltage = readMux(module[i].batteryVolatgePin); // Get battery voltage
			sprintf_P(serialSendString + strlen(serialSendString), PSTR("&CS%d=2&TI%d=%d&IT%d=%d&IV%d=%d.%02d&CT%d=%d&CV%d=%d.%02d&HT%d=%d"), i, i, (module[i].seconds + (module[i].minutes * 60) + (module[i].hours * 3600)), i, module[i].batteryInitialTemp, i, (int)module[i].batteryInitialVoltage, (int)(module[i].batteryInitialVoltage * 100) % 100, i, module[i].batteryCurrentTemp, i, (int)module[i].batteryVoltage, (int)(module[i].batteryVoltage * 100) % 100, i, module[i].batteryHighestTemp);
			if (processTemperature(i) == 2)
			{
				//Battery Temperature is >= MAX Threshold considered faulty
				digitalSwitch(module[i].chargeMosfetPin, 0); // Turn off TP4056
				module[i].batteryFaultCode = 7;				 // Set the Battery Fault Code to 7 High Temperature
				if (module[i].insertData == true)
				{
					clearSecondsTimer(i);
					module[i].insertData = false;
					module[i].cycleState = 7; // Temperature is to high. Battery is considered faulty set cycleState to Completed
					module[i].cycleCount = 0; // Reset cycleCount for use in other Cycles
				}
				sprintf_P(serialSendString + strlen(serialSendString), PSTR("&ID%d"), i);
			}
			else
			{
				digitalSwitch(module[i].chargeMosfetPin, 1); // Turn on TP4056
				module[i].cycleCount = module[i].cycleCount + chargeCycle(i);
				if (module[i].cycleCount >= 5)
				{
					digitalSwitch(module[i].chargeMosfetPin, 0); // Turn off TP4056
					if (module[i].insertData == true)
					{
						clearSecondsTimer(i);
						module[i].insertData = false;
						module[i].cycleState = 3; // Charge Battery Completed set cycleState to Check Battery Milli Ohms
						module[i].cycleCount = 0; // Reset cycleCount for use in other Cycles
					}
					sprintf_P(serialSendString + strlen(serialSendString), PSTR("&ID%d"), i);
				}
			}
			if (module[i].hours == settings.chargingTimeout) // Charging has reached Timeout period. Either battery will not hold charge, has high capacity or the TP4056 is faulty
			{
				digitalSwitch(module[i].chargeMosfetPin, 0); // Turn off TP4056
				module[i].batteryFaultCode = 9;				 // Set the Battery Fault Code to 7 Charging Timeout
				if (module[i].insertData == true)
				{
					clearSecondsTimer(i);
					module[i].insertData = false;
					module[i].cycleState = 7; // Charging Timeout. Battery is considered faulty set cycleState to Completed
					module[i].cycleCount = 0; // Reset cycleCount for use in other Cycles
				}
				sprintf_P(serialSendString + strlen(serialSendString), PSTR("&ID%d"), i);
			}
			break;
		case 3: // Check Battery Milli Ohms
			module[i].cycleCount = module[i].cycleCount + milliOhms(i);
			module[i].tempMilliOhmsValue = module[i].tempMilliOhmsValue + module[i].milliOhmsValue;
			if (module[i].cycleCount == 4)
			{
				module[i].milliOhmsValue = module[i].tempMilliOhmsValue / 4;
				if (module[i].milliOhmsValue > settings.highMilliOhms) // Check if Milli Ohms is greater than the set high Milli Ohms value
				{
					module[i].batteryFaultCode = 3; // Set the Battery Fault Code to 3 High Milli Ohms
					module[i].cycleState = 7;		// Milli Ohms is high battery is considered faulty set cycleState to Completed
					module[i].cycleCount = 0;		// Reset cycleCount for use in other Cycles
				}
				else
				{
					if (module[i].minutes <= 1) // No need to rest the battery if it is already charged
					{
						module[i].cycleState = 5; // Check Battery Milli Ohms Completed set cycleState to Discharge Battery
						module[i].cycleCount = 0; // Reset cycleCount for use in other Cycles
					}
					else
					{
						module[i].cycleState = 4; // Check Battery Milli Ohms Completed set cycleState to Rest Battery
						module[i].cycleCount = 0; // Reset cycleCount for use in other Cycles
					}
					clearSecondsTimer(i);
				}
			}
			sprintf_P(serialSendString + strlen(serialSendString), PSTR("&CS%d=3&MO%d=%d&CV%d=%d.%02d"), i, i, (int)module[i].milliOhmsValue, i, (int)module[i].batteryVoltage, (int)(module[i].batteryVoltage * 100) % 100);
			break;

		case 4:																 // Rest Battery
			module[i].batteryVoltage = readMux(module[i].batteryVolatgePin); // Get battery voltage
			module[i].batteryCurrentTemp = getTemperature(i);
			if (module[i].minutes == settings.restTimeMinutes) // Rest time
			{
				module[i].batteryInitialVoltage = module[i].batteryVoltage; // Reset Initial voltage
				clearSecondsTimer(i);
				module[i].cycleState = 5; // Rest Battery Completed set cycleState to Discharge Battery
			}
			sprintf_P(serialSendString + strlen(serialSendString), PSTR("&CS%d=4&TI%d=%d&CT%d=%d&CV%d=%d.%02d"), i, i, (module[i].seconds + (module[i].minutes * 60) + (module[i].hours * 3600)), i, module[i].batteryCurrentTemp, i, (int)module[i].batteryVoltage, (int)(module[i].batteryVoltage * 100) % 100);
			break;
		case 5: // Discharge Battery
			sprintf_P(serialSendString + strlen(serialSendString), PSTR("&CS%d=5&TI%d=%d&IT%d=%d&IV%d=%d.%02d&CT%d=%d&CV%d=%d.%02d&HT%d=%d&MA%d=%d&DA%d=%d.%02d&MO%d=%d"), i, i, (module[i].seconds + (module[i].minutes * 60) + (module[i].hours * 3600)), i, module[i].batteryInitialTemp, i, (int)module[i].batteryInitialVoltage, (int)(module[i].batteryInitialVoltage * 100) % 100, i, module[i].batteryCurrentTemp, i, (int)module[i].dischargeVoltage, (int)(module[i].dischargeVoltage * 100) % 100, i, module[i].batteryHighestTemp, i, (int)module[i].dischargeMilliamps, i, (int)module[i].dischargeAmps, (int)(module[i].dischargeAmps * 100) % 100, i, (int)module[i].milliOhmsValue);
			if (processTemperature(i) == 2)
			{
				//Battery Temperature is >= MAX Threshold considered faulty
				digitalSwitch(module[i].dischargeMosfetPin, 0); // Turn off Discharge Mosfet
				module[i].batteryFaultCode = 7;					// Set the Battery Fault Code to 7 High Temperature
				if (module[i].insertData == true)
				{
					clearSecondsTimer(i);
					module[i].insertData = false;
					module[i].cycleState = 7; // Temperature is high. Battery is considered faulty set cycleState to Completed
				}
				sprintf_P(serialSendString + strlen(serialSendString), PSTR("&ID%d"), i);
			}
			else
			{
				if (module[i].dischargeCompleted == true)
				{
					sprintf_P(serialSendString + strlen(serialSendString), PSTR("&ID%d"), i);
					if (module[i].dischargeMilliamps < settings.lowMilliamps) // No need to recharge the battery if it has low Milliamps
					{
						module[i].batteryFaultCode = 5; // Set the Battery Fault Code to 5 Low Milliamps
						if (module[i].insertData == true)
						{
							clearSecondsTimer(i);
							module[i].insertData = false;
							module[i].cycleState = 7; // Discharge Battery Completed set cycleState to Completed
						}
					}
					else
					{
						module[i].batteryVoltage = readMux(module[i].batteryVolatgePin); // Get battery voltage for Recharge Cycle
						module[i].batteryInitialVoltage = module[i].batteryVoltage;		 // Reset Initial voltage
						if (module[i].insertData == true)
						{
							clearSecondsTimer(i);
							module[i].insertData = false;
							module[i].cycleState = 6; // Discharge Battery Completed set cycleState to Recharge Battery
						}
					}
				}
				else
				{
					if (dischargeCycle(i))
						module[i].dischargeCompleted = true;
				}
			}
			break;
		case 6:																 // Recharge Battery
			module[i].batteryVoltage = readMux(module[i].batteryVolatgePin); // Get battery voltage
			sprintf_P(serialSendString + strlen(serialSendString), PSTR("&CS%d=6&TI%d=%d&IT%d=%d&IV%d=%d.%02d&CT%d=%d&CV%d=%d.%02d&HT%d=%d"), i, i, (module[i].seconds + (module[i].minutes * 60) + (module[i].hours * 3600)), i, module[i].batteryInitialTemp, i, (int)module[i].batteryInitialVoltage, (int)(module[i].batteryInitialVoltage * 100) % 100, i, module[i].batteryCurrentTemp, i, (int)module[i].batteryVoltage, (int)(module[i].batteryVoltage * 100) % 100, i, module[i].batteryHighestTemp);
			if (processTemperature(i) == 2)
			{
				//Battery Temperature is >= MAX Threshold considered faulty
				digitalSwitch(module[i].chargeMosfetPin, 0); // Turn off TP4056
				module[i].batteryFaultCode = 7;				 // Set the Battery Fault Code to 7 High Temperature
				if (module[i].insertData == true)
				{
					clearSecondsTimer(i);
					module[i].insertData = false;
					module[i].cycleState = 7; // Temperature is to high. Battery is considered faulty set cycleState to Completed
					module[i].cycleCount = 0; // Reset cycleCount for use in other Cycles
				}
				sprintf_P(serialSendString + strlen(serialSendString), PSTR("&ID%d"), i);
			}
			else
			{
				digitalSwitch(module[i].chargeMosfetPin, 1); // Turn on TP4056
				module[i].cycleCount = module[i].cycleCount + chargeCycle(i);
				if (module[i].cycleCount >= 5)
				{
					digitalSwitch(module[i].chargeMosfetPin, 0); // Turn off TP4056
					if (module[i].insertData == true)
					{
						clearSecondsTimer(i);
						module[i].insertData = false;
						module[i].cycleState = 7; // Recharge Battery Completed set cycleState to Completed
						module[i].cycleCount = 0; // Reset cycleCount for use in other Cycles
					}
					sprintf_P(serialSendString + strlen(serialSendString), PSTR("&ID%d"), i);
				}
			}
			if (module[i].hours == settings.chargingTimeout) // Charging has reached Timeout period. Either battery will not hold charge, has high capacity or the TP4056 is faulty
			{
				digitalSwitch(module[i].chargeMosfetPin, 0); // Turn off TP4056
				module[i].batteryFaultCode = 9;				 // Set the Battery Fault Code to 7 Charging Timeout
				if (module[i].insertData == true)
				{
					clearSecondsTimer(i);
					module[i].insertData = false;
					module[i].cycleState = 7; // Charging Timeout. Battery is considered faulty set cycleState to Completed
					module[i].cycleCount = 0; // Reset cycleCount for use in other Cycles
				}
				sprintf_P(serialSendString + strlen(serialSendString), PSTR("&ID%d"), i);
			}
			break;
		case 7: // Completed
			if (!batteryCheck(i))
				module[i].cycleCount++;
			if (module[i].cycleCount == 2)
			{
				module[i].cycleState = 0; // Completed and Battery Removed set cycleState to Check Battery Voltage
				module[i].cycleCount = 0; // Reset cycleCount for use in other Cycles
			}
			sprintf_P(serialSendString + strlen(serialSendString), PSTR("&CS%d=7&CV%d=%d.%02d&FC%d=%d"), i, i, (int)module[i].batteryVoltage, (int)(module[i].batteryVoltage * 100) % 100, i, module[i].batteryFaultCode);
			break;
		}
		secondsTimer(i);
	}
	cycleStateLCD();
	fanController();
}

void cycleStateLCDOutput(byte j)
{
	char lcdLine0[20];
	char lcdLine1[20];

	switch (module[j].cycleState)
	{
	case 0: // Check Battery Voltage
		sprintf_P(lcdLine0, PSTR("%d%-15S"), j + 1, PSTR("-BATTERY CHECK"));
		sprintf_P(lcdLine1, PSTR("%-11S%d.%02dV"), module[j].cycleCount > 0 ? PSTR("DETECTED") : PSTR("INSERT BAT"), (int)module[j].batteryVoltage, (int)(module[j].batteryVoltage * 100) % 100);
		break;
	case 1: // Get Battery Barcode
		sprintf_P(lcdLine0, PSTR("%d%-15S"), j + 1, PSTR("-SCAN BARCODE"));
		sprintf_P(lcdLine1, PSTR("%-11S%d.%02dV"), PSTR(" "), (int)module[j].batteryVoltage, (int)(module[j].batteryVoltage * 100) % 100);
		break;
	case 2: // Charge Battery
		sprintf_P(lcdLine0, PSTR("%d%-7S%02d:%02d:%02d"), j + 1, PSTR("-CHRG "), module[j].hours, module[j].minutes, module[j].seconds);
		sprintf_P(lcdLine1, PSTR("%d.%02dV  %02d%c %d.%02dV"), (int)module[j].batteryInitialVoltage, (int)(module[j].batteryInitialVoltage * 100) % 100, module[j].batteryCurrentTemp, 223, (int)module[j].batteryVoltage, (int)(module[j].batteryVoltage * 100) % 100);
		break;
	case 3: // Check Battery Milli Ohms
		sprintf_P(lcdLine0, PSTR("%d%-15S"), j + 1, PSTR("-RESISTANCE"));
		sprintf_P(lcdLine1, PSTR("%-10S%04dm%c"), PSTR("MILLIOHMS"), (int)module[j].milliOhmsValue, 244);
		break;
	case 4: // Rest Battery
		sprintf_P(lcdLine0, PSTR("%d%-7S%02d:%02d:%02d"), j + 1, PSTR("-REST"), module[j].hours, module[j].minutes, module[j].seconds);
		sprintf_P(lcdLine1, PSTR("%-11S%d.%02dV"), PSTR(" "), (int)module[j].batteryVoltage, (int)(module[j].batteryVoltage * 100) % 100);
		break;
	case 5: // Discharge Battery
		sprintf_P(lcdLine0, PSTR("%d%-4S%d.%02dA %d.%02dV"), j + 1, PSTR("-DC"), (int)module[j].dischargeAmps, (int)(module[j].dischargeAmps * 100) % 100, (int)module[j].dischargeVoltage, (int)(module[j].dischargeVoltage * 100) % 100);
		sprintf_P(lcdLine1, PSTR("%02d:%02d:%02d %04dmAh"), module[j].hours, module[j].minutes, module[j].seconds, (int)module[j].dischargeMilliamps);
		break;
	case 6: // Recharge Battery
		sprintf_P(lcdLine0, PSTR("%d%-7S%02d:%02d:%02d"), j + 1, PSTR("-RCHG "), module[j].hours, module[j].minutes, module[j].seconds);
		sprintf_P(lcdLine1, PSTR("%d.%02dV  %02d%c %d.%02dV"), (int)module[j].batteryInitialVoltage, (int)(module[j].batteryInitialVoltage * 100) % 100, module[j].batteryCurrentTemp, 223, (int)module[j].batteryVoltage, (int)(module[j].batteryVoltage * 100) % 100);
		break;
	case 7: // Completed
		switch (module[j].batteryFaultCode)
		{
		case 0: // Finished
			sprintf_P(lcdLine0, PSTR("%d%-15S"), j + 1, PSTR("-FINISHED"));
			break;
		case 3: // High Milli Ohms
			sprintf_P(lcdLine0, PSTR("%d%-15S"), j + 1, PSTR("-FAULT HIGH OHM"));
			break;
		case 5: // Low Milliamps
			sprintf_P(lcdLine0, PSTR("%d%-15S"), j + 1, PSTR("-FAULT LOW mAh"));
			break;
		case 7: // High Temperature
			sprintf_P(lcdLine0, PSTR("%d%-15S"), j + 1, PSTR("-FAULT HIGH TMP"));
			break;
		case 9: // Charge Timeout
			sprintf_P(lcdLine0, PSTR("%d%-15S"), j + 1, PSTR("-FAULT CHG TIME"));
			break;
		}
		sprintf_P(lcdLine1, PSTR("%04dm%c   %04dmAh"), (int)module[j].milliOhmsValue, 244, (int)module[j].dischargeMilliamps);
		break;
	}
	lcd.setCursor(0, 0);
	lcd.print(lcdLine0);
	lcd.setCursor(0, 1);
	lcd.print(lcdLine1);
}

bool dischargeCycle(byte j)
{
	float batteryShuntVoltage = 0.00;
	module[j].intMilliSecondsCount = module[j].intMilliSecondsCount + (millis() - module[j].longMilliSecondsPreviousCount);
	module[j].longMilliSecondsPreviousCount = millis();
	if (module[j].intMilliSecondsCount >= settings.dischargeReadInterval || module[j].dischargeAmps == 0) // Get reading every 5+ seconds or if dischargeAmps = 0 then it is first run
	{
		module[j].dischargeVoltage = readMux(module[j].batteryVolatgePin);
		batteryShuntVoltage = readMux(module[j].batteryVolatgeDropPin);
		if (module[j].dischargeVoltage >= settings.defaultBatteryCutOffVoltage)
		{
			digitalSwitch(module[j].dischargeMosfetPin, 1); // Turn on Discharge Mosfet
			module[j].dischargeAmps = (module[j].dischargeVoltage - batteryShuntVoltage) / settings.shuntResistor[j];
			module[j].longMilliSecondsPassed = millis() - module[j].longMilliSecondsPrevious;
			module[j].dischargeMilliamps = module[j].dischargeMilliamps + (module[j].dischargeAmps * 1000.0) * (module[j].longMilliSecondsPassed / 3600000.0);
			module[j].longMilliSecondsPrevious = millis();
		}
		module[j].intMilliSecondsCount = 0;
		if (module[j].dischargeVoltage < settings.defaultBatteryCutOffVoltage)
		{
			digitalSwitch(module[j].dischargeMosfetPin, 0); // Turn off Discharge Mosfet
			return true;
		}
	}
	return false;
}

byte milliOhms(byte j)
{
	float resistanceAmps = 0.00;
	float voltageDrop = 0.00;
	float batteryVoltageInput = 0.00;
	float batteryShuntVoltage = 0.00;
	digitalSwitch(module[j].dischargeMosfetPin, 0); // Turn off the Discharge Mosfet
	batteryVoltageInput = readMux(module[j].batteryVolatgePin);
	digitalSwitch(module[j].dischargeMosfetPin, 1); // Turn on the Discharge Mosfet
	batteryShuntVoltage = readMux(module[j].batteryVolatgePin);
	digitalSwitch(module[j].dischargeMosfetPin, 0); // Turn off the Discharge Mosfet
	resistanceAmps = batteryShuntVoltage / settings.shuntResistor[j];
	voltageDrop = batteryVoltageInput - batteryShuntVoltage;
	module[j].milliOhmsValue = ((voltageDrop / resistanceAmps) * 1000) + settings.offsetMilliOhms; // The Drain-Source On-State Resistance of the Discharge Mosfet
	if (module[j].milliOhmsValue > 9999)
		module[j].milliOhmsValue = 9999;
	return 1;
}

bool chargeCycle(byte j)
{
	if (readMux(module[j].chargeLedPin) >= 1.8) // Need to define this in Settings
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

byte processTemperature(byte j)
{
	module[j].batteryCurrentTemp = getTemperature(j);
	if (module[j].batteryCurrentTemp > module[j].batteryHighestTemp)
		module[j].batteryHighestTemp = module[j].batteryCurrentTemp; // Set highest temperature if current value is higher
	if ((module[j].batteryCurrentTemp - ambientTemperature) > settings.tempThreshold)
	{
		if ((module[j].batteryCurrentTemp - ambientTemperature) > settings.tempMaxThreshold)
		{
			//Temp higher than Maximum Threshold
			return 2;
		}
		else
		{
			//Temp higher than Threshold <- Does nothing yet need some flag / warning
			return 1;
		}
	}
	else
	{
		//Temp lower than Threshold
		return 0;
	}
}

int getTemperature(byte j)
{
	if (module[j].tempCount > 16 || module[j].batteryCurrentTemp == 0) // Read every 16x cycles
	{
		module[j].tempCount = 0;
		sensors.requestTemperaturesByAddress(tempSensorSerial[j]);
		float tempC = sensors.getTempC(tempSensorSerial[j]);
		if (tempC > 99 || tempC < 0)
			tempC = 99;
		return (int)tempC;
	}
	else
	{
		module[j].tempCount++;
		return module[j].batteryCurrentTemp;
	}
}

void getAmbientTemperature()
{
	static byte ambientTempCount;
	if (ambientTempCount > 16 || ambientTemperature == 0) // Read every 16x cycles
	{
		ambientTempCount = 0;
		sensors.requestTemperaturesByAddress(tempSensorSerial[4]);
		float tempC = sensors.getTempC(tempSensorSerial[4]);
		if (tempC > 99 || tempC < 0)
			tempC = 99;
		ambientTemperature = tempC;
	}
	else
	{
		ambientTempCount++;
	}
}

bool batteryCheck(byte j)
{
	module[j].batteryVoltage = readMux(module[j].batteryVolatgePin);
	if (module[j].batteryVoltage <= settings.batteryVolatgeLeak)
	{
		return false;
	}
	else
	{
		return true;
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
		batterySampleVoltage = batterySampleVoltage + analogRead(SIG);
	}
	batterySampleVoltage = batterySampleVoltage / 10;
	return batterySampleVoltage * settings.referenceVoltage / 1023.0; // Calculate and return the Voltage Reading
}

void returnCodes(int codeID)
{
	switch (codeID)
	{
	case 0: // SUCCESSFUL
		Serial.println(F("SUCCESSFUL"));
		break;
	case 1: // CONNECTION ERROR
		Serial.println(F("CONNECTION_ERROR"));
		break;
	case 2: // TIMEOUT
		Serial.println(F("TIMEOUT"));
		break;
	case 3: // ERROR_DATABASE
		Serial.println(F("ERROR_DATABASE"));
		break;
	case 4: // ERROR_MISSING_DATA
		Serial.println(F("ERROR_MISSING_DATA"));
		break;
	case 5: // ERROR_NO_BARCODE_DB
		Serial.println(F("ERROR_NO_BARCODE_DB"));
		break;
	case 6: // ERROR_NO_BARCODE_INPUT
		Serial.println(F("ERROR_NO_BARCODE_INPUT"));
		break;
	case 7: // ERROR_DATABASE_HASH_INPUT
		Serial.println(F("ERROR_DATABASE_HASH_INPUT"));
		break;
	case 8: // ERROR_HASH_INPUT
		Serial.println(F("ERROR_HASH_INPUT"));
		break;
	case 9: // ERROR_SERIAL_OUTPUT
		Serial.println(F("ERROR_SERIAL_OUTPUT"));
		break;
	case 100: // BARCODE_CONTINUE_0
		module[0].batteryBarcode = true;
		Serial.println(F("BARCODE_CONTINUE_0"));
		break;
	case 101: // BARCODE_CONTINUE_1
		module[1].batteryBarcode = true;
		Serial.println(F("BARCODE_CONTINUE_1"));
		break;
	case 102: // BARCODE_CONTINUE_2
		module[2].batteryBarcode = true;
		Serial.println(F("BARCODE_CONTINUE_2"));
		break;
	case 103: // BARCODE_CONTINUE_3
		module[3].batteryBarcode = true;
		Serial.println(F("BARCODE_CONTINUE_3"));
		break;
	case 200: // INSERT_DATA_SUCCESSFUL_0
		module[0].insertData = true;
		Serial.println(F("INSERT_DATA_SUCCESSFUL_0"));
		break;
	case 201: // INSERT_DATA_SUCCESSFUL_1
		module[1].insertData = true;
		Serial.println(F("INSERT_DATA_SUCCESSFUL_1"));
		break;
	case 202: // INSERT_DATA_SUCCESSFUL_2
		module[2].insertData = true;
		Serial.println(F("INSERT_DATA_SUCCESSFUL_2"));
		break;
	case 203: // INSERT_DATA_SUCCESSFUL_3
		module[3].insertData = true;
		Serial.println(F("INSERT_DATA_SUCCESSFUL_3"));
		break;
	default:
		Serial.println(F("UKNOWN"));
		break;
	}
}