Arduino-Nano-4x-PCB-Smart-Charger-Discharger
---------------------------------------------------------------------------
Originally created by Brett Watt on 03/01/2019
Copyright 2018 - Under creative commons license 3.0:

This software is furnished "as is", without technical support, and with no 
warranty, express or implied, as to its usefulness for any purpose.
 
This is the Arduino Nano 4x 18650 Smart Charger / Discharger Code

Email: info@vortexit.co.nz 
Web: www.vortexit.co.nz

---------------------------------------------------------------------------

This sketch require you to have VS-Code installed, and plugin Github (usually installed by default) and Platform IO installed:
1. Download and install VS code (https://code.visualstudio.com/download)
2. Install PlatformIO plugin (https://platformio.org/install/ide?install=vscode)

Then clone repository:
3. Copy link to the repository (green "Code" button top left of github site)
4. In VS Code, go to View->Command Pallet (or press CTRL+SHIFT+P) and type "Git: Clone". Press Enter
5. Paste the link and press Enter. Choose a directory to save the files.
6. Click File->Open Workspace, and open the file "ASCD Nano.code-workspace" from the folder you just cloned.

---------------------------------------------------------------------------

Upload test-files to Arduino Nano:
1. Connect the chip to the system.
2. Under "ASCD Nano (Workspace)" to the left, open the file "ASCD Nano"->"platformio.ini"
3. All the way to the left select the Platform IO symbol (the Ant symbol)
4. Under "Project Task", choose either "nanoatmega328new" or "nanoatmega328old", depending on your chip, and select the test you need to execute.
5. Click Upload and monitor. PlatformIO will download the required libraries, compile and upload the sketch.

---------------------------------------------------------------------------

Upload sketch to Arduino Nano:
1. Connect the chip to the system.
2. Under "ASCD Nano (Workspace)" to the left, open the file "ASCD Nano"->"platformio.ini"
3. All the way to the left select the Platform IO symbol (the Ant symbol)
4. Under "Project Task", select either "nanoatmega328new-main" or "nanoatmega328old-main", depending on your chip.
5. Click Upload. PlatformIO will download the required libraries, compile and upload the sketch.

---------------------------------------------------------------------------

Configure and upload sketch to ESP8266 ESP-01:

1. Connect the chip to the system with the switch on PROG.
2. Under "ASCD Nano (Workspace)" to the left, open the file "ESP8266 Wifi Client"->"src"->"main.cpp".
3. Change:
const char ssid[] = ""; -> to your WIFI routers SSID
const char password[] = "";-> to your WIFI routers Password
const char userHash[] = ""; -> to your UserHash (Get this from "Charger / Discharger Menu -> View" in the Vortex It Battery Portal)
const byte CDUnitID = ; -> to your CDUnitID (Get this from "Charger / Discharger Menu -> View -> Select your Charger / Discharger" in the Vortex It Battery Portal)
4. Close the file, and open "platform.ini" under "ESP8266 Wifi Client".
5. All the way to the left select the Platform IO symbol (the Ant symbol)
6. Under "Project Task", click "env:esp01_1m" (1mega memory) or "env:esp01" (512kilo memory), depending on your chip.
7. Click Upload. PlatformIO will download the required libraries, compile and upload the sketch.

---------------------------------------------------------------------------

Included Libraries:
DallasTemperature.h
OneWire.h - https://github.com/milesburton/Arduino-Temperature-Control-Library
LiquidCrystal_I2C.h - https://github.com/marcoschwartz/LiquidCrystal_I2C
