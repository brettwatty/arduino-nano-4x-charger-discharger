// ASDC Nano 4x Arduino Charger / Discharger
// ---------------------------------------------------------------------------
// Created by Brett Watt on 19/03/2019
// Copyright 2019 - Under creative commons license 3.0:
//
// This software is furnished "as is", without technical support, and with no
// warranty, express or implied, as to its usefulness for any purpose.
//
// @brief
// ASDC Nano 4x Arduino Charger / Discharger
// Code for getting the Dallas DS18B20 Serials
// Version 1.0.0
//
// @author Email: info@vortexit.co.nz
//       Web: www.vortexit.co.nz

// Include the libraries we need
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4 // Pin 4 for PCB Version 1.11+
#define TEMPERATURE_PRECISION 9

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress tempSensorSerial[5];

byte deviceCount = 5; // 9 in the ASDC Mega 8x Arduino Charger / Discharger
float sensorTempAverage = 0;
bool tempSensorSerialCompleted[10];
bool detectionComplete = false;
byte tempSensorSerialOutput[5]; //Sensors 1 - 5
byte pendingDetection = 0;      // This will be from Battery 1 to 4 and then 5 for the ambient temperature

//Function declaration
void printAddress(DeviceAddress deviceAddress, bool first, bool last, bool comma);
byte useOneWireSearch();
void printTemperature(DeviceAddress deviceAddress);
void printResolution(DeviceAddress deviceAddress);

void setup(void)
{
  // start serial port
  Serial.begin(115200);
  Serial.println("Dallas Temperature Detection");

  // Start up the library
  sensors.begin();

  // Locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  if (sensors.getDeviceCount() != deviceCount)
  {
    Serial.println("");
    Serial.print("ERROR did no detect all the Sensors ");
    Serial.print(sensors.getDeviceCount(), DEC);
    Serial.print(" found. ");
    Serial.print(deviceCount);
    Serial.println(" Sensors should exist.");

    Serial.println("");
    Serial.println("Atempting to use generic One Wire search to find non genuine Dallas DS18B20 sensors......");
    Serial.print("Found ");
    byte foundCount = useOneWireSearch();
    Serial.print(foundCount);
    Serial.println(" devices using One Wire Search.");
    if (foundCount < deviceCount)
    {
      Serial.println("");
      Serial.print("ERROR did no detect all the Sensors using One Wire Search ");
      Serial.print(foundCount);
      Serial.print(" found. ");
      Serial.print(deviceCount);
      Serial.println(" Sensors should exist.");
      while (1)
        ;
    }
  }

  // oneWire.reset_search();

  byte i = 0;
  byte address[8];
  if (oneWire.search(tempSensorSerial[i]))
  {
    do
    {
      delay(2000);
      Serial.print("Device ");
      Serial.print(i);
      Serial.print(" Address: ");
      printAddress(tempSensorSerial[i], false, false, false);
      Serial.println();
      sensors.setResolution(tempSensorSerial[i], TEMPERATURE_PRECISION);
      i++;
    } while (oneWire.search(tempSensorSerial[i]));
  }

  sensors.requestTemperatures();
  for (byte i = 0; i < deviceCount; i++)
  {
    sensorTempAverage += sensors.getTempC(tempSensorSerial[i]);
  }
  sensorTempAverage = sensorTempAverage / deviceCount;
}

void loop(void)
{
  if (detectionComplete == false)
  {
    //Serial.print("Average Temp: ");
    //Serial.println(sensorTempAverage);
    Serial.print("-------------------------------------");
    Serial.print("Heat Up Battery Sensor: ");
    Serial.print(pendingDetection + 1);
    Serial.println("-------------------------------------");
    sensors.requestTemperatures();
    for (byte i = 0; i < deviceCount; i++)
    {
      if (tempSensorSerialCompleted[i] == false)
      {
        if (pendingDetection != (deviceCount - 1))
        {
          Serial.print("Sensor ");
          Serial.print(i);
          Serial.print(" Temp: ");
          Serial.println(sensors.getTempC(tempSensorSerial[i]));
          if (sensors.getTempC(tempSensorSerial[i]) > (2.5 + sensorTempAverage))
          {
            Serial.print("Detected Battery: ");
            Serial.println(pendingDetection + 1);
            tempSensorSerialCompleted[i] = true;
            tempSensorSerialOutput[pendingDetection] = i;
            pendingDetection++; //If not greater than number of devices - last one = ambiant
          }
        }
        else
        {
          Serial.println("");
          Serial.println("");
          Serial.println("");
          Serial.println("");
          Serial.print("-------------------------------------");
          Serial.print("Detected Ambient Sensor Completed");
          Serial.println("-------------------------------------");
          tempSensorSerialCompleted[i] = true;
          // Got the last one we are done
          tempSensorSerialOutput[pendingDetection] = i;
          detectionComplete = true;
        }
      }
    }
    delay(4000);
  }
  else
  {
    //else show serials
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("-------------------------------------");
    Serial.println("Copy and Paste these Addresses into the Arduino Charger / Discharger Sketch");
    Serial.println("-------------------------------------");
    for (byte i = 0; i < deviceCount; i++)
    {
      if (i == 0)
      {
        printAddress(tempSensorSerial[tempSensorSerialOutput[i]], true, false, true);
      }
      else if (i == (deviceCount - 1))
      {
        printAddress(tempSensorSerial[tempSensorSerialOutput[i]], false, true, false);
      }
      else
      {
        printAddress(tempSensorSerial[tempSensorSerialOutput[i]], false, false, true);
      }
    }
    Serial.println("-------------------------------------");
    while (1)
      ;
  }
}

// Function to print a device address
void printAddress(DeviceAddress deviceAddress, bool first, bool last, bool comma)
{
  if (first)
  {
    Serial.println("DeviceAddress tempSensorSerial[5]= {{");
  }
  else
  {
    Serial.print("{");
  }
  for (byte i = 0; i < 8; i++)
  {
    Serial.print("0x");
    if (deviceAddress[i] < 0x10)
      Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    if (i < 7)
      Serial.print(", ");
  }
  if (last)
  {
    Serial.println("}};");
  }
  else
  {
    if (comma)
    {
      Serial.println("},");
    }
    else
    {
      Serial.print("}");
    }
  }
}

byte useOneWireSearch()
{
  byte i = 0;
  byte address[8];
  if (oneWire.search(address))
  {
    do
    {
      i++;
    } while (oneWire.search(address));
  }
  return i;
}

// Function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  Serial.print("Temp C: ");
  Serial.print(tempC);
  //Serial.print(" Temp F: ");
  //Serial.print(DallasTemperature::toFahrenheit(tempC));
}

// Function to print a device's resolution
void printResolution(DeviceAddress deviceAddress)
{
  Serial.print("Resolution: ");
  Serial.print(sensors.getResolution(deviceAddress));
  Serial.println();
}