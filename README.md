Arduino-Nano-4x-PCB-Smart-Charger-Discharger
---------------------------------------------------------------------------
Created by Brett Watt on 03/01/2019
Copyright 2018 - Under creative commons license 3.0:

This software is furnished "as is", without technical support, and with no 
warranty, express or implied, as to its usefulness for any purpose.
 
This is the Arduino Nano 4x 18650 Smart Charger / Discharger Code

Email: info@vortexit.co.nz 
Web: www.vortexit.co.nz

---------------------------------------------------------------------------

Arduino Nano 3.0 ATmega328P:

Upload sketch "ASCD_Nano_1-0-0.ino" to the Arduino Nano

You may need to use ATmega328P (Old boot loader) as the processor in Arduino IDE

---------------------------------------------------------------------------

ESP8266 ESP-01:

You need to install the ESP8266 Arduino Addon in your Arduino IDE use this guide:

https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/installing-the-esp8266-arduino-addon

Change-
const char ssid[] = ""; -> to your WIFI routers SSID
const char password[] = "";-> to your WIFI routers Password
const char userHash[] = ""; -> to your UserHash (Get this from "Charger / Discharger Menu -> View" in the Vortex It Battery Portal)
const byte CDUnitID = ; -> to your CDUnitID (Get this from "Charger / Discharger Menu -> View -> Select your Charger / Discharger" in the Vortex It Battery Portal)

Use USB to ESP8266 ESP-01 Programmer to upload sketch ESP8266_Wifi_Client_01.ino to the ESP8266 with the switch on PROG 

---------------------------------------------------------------------------

Included Libraries:
DallasTemperature.h, OneWire.h - https://github.com/milesburton/Arduino-Temperature-Control-Library
LiquidCrystal_I2C.h - https://github.com/marcoschwartz/LiquidCrystal_I2C