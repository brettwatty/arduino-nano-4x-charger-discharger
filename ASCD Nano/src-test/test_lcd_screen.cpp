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
// Code for testing the 16x2 LCD 
// Version 1.0.0
//
// @author Email: info@vortexit.co.nz
//       Web: www.vortexit.co.nz

#include <Arduino.h>
#include <LiquidCrystal_I2C.h> 
LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup() 
{
  lcd.init();
  lcd.clear();
  lcd.backlight();// Turn on backlight
  lcd.setCursor(0,0);
  lcd.print("Testing LCD 16x2");
  lcd.setCursor(0,1);
  lcd.print("0123456789ABCDEF");
}
  
void loop()
{
  
}