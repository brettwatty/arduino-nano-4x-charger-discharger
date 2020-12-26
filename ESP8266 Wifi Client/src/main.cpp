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
// Main code for the ESP8266 ESP-01
// Version 1.0.0
//
// @author Email: info@vortexit.co.nz
//       Web: www.vortexit.co.nz

#include <ESP8266WiFi.h>

// Wifi Variables
const char ssid[] = "";                        // SSID
const char password[] = "";                    // Password
const char server[] = "submit.vortexit.co.nz"; // Server to connect to send and recieve data
const char userHash[] = "";                    // Database Hash - this is unique per user - Get this from Charger / Discharger Menu -> View
const int CDUnitID = 0;                       // CDUnitID this is the Units ID - this is unique per user - Get this from Charger / Discharger Menu -> View -> Select your Charger / Discharger

// readPage Variables
char serverResult[32];  // String for incoming serial data
int stringPosition = 0; // String index counter readPage()
bool startRead = false; // Is reading? readPage()

String updateUnitDataString = "";

WiFiClient client;

//Function declaration
String updateUnitData();
String readPage();

void setup()
{
  Serial.begin(57600);
  Serial.setTimeout(5);
  //Serial.print("Connecting to ");
  //Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  //Serial.println("");
  //Serial.println("WiFi connected");
  //Serial.println("IP address: ");
  //Serial.println(WiFi.localIP());
}

void loop()
{
  if (updateUnitDataString != "")
  {
    String resultUpdateUnitData = updateUnitData();
    updateUnitDataString = "";
    Serial.println(resultUpdateUnitData);
  }
  else
  {
    while (Serial.available())
    {
      updateUnitDataString = Serial.readString(); // read the incoming data as string
      updateUnitDataString.trim();
    }
  }
}

String updateUnitData()
{
  if (client.connect(server, 80))
  {
    client.print("GET /update_unit_data.php?");
    client.print("UH=");
    client.print(userHash);
    client.print("&");
    client.print("CD=");
    client.print(CDUnitID);
    client.print(updateUnitDataString);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    client.println();
    client.println();
    return readPage();
  }
  else
  {
    return "1";
  }
}

String readPage()
{
  stringPosition = 0;
  unsigned long startTime = millis();
  memset(&serverResult, 0, 32); //Clear serverResult memory
  while (true)
  {
    if (millis() < startTime + 3750) // Time out of 3750 milliseconds (Possible cause of crash on Ethernet)
    {
      if (client.available())
      {
        char c = client.read();
        if (c == '<') //'<' Start character
        {
          startRead = true; //Ready to start reading the part
        }
        else if (startRead)
        {
          if (c != '>') //'>' End character
          {
            serverResult[stringPosition] = c;
            stringPosition++;
          }
          else
          {
            startRead = false;
            client.stop();
            client.flush();
            return String(serverResult);
          }
        }
      }
    }
    else
    {
      client.stop();
      client.flush();
      return "2"; //TIMEOUT
    }
  }
}
