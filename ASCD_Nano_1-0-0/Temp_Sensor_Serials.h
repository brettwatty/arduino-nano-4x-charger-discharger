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
// Dallas Temperature Sensor Serial header file
// Version 1.0.0
//
// @author Email: info@vortexit.co.nz
//       Web: www.vortexit.co.nz

// You need to run the Dallas get temp sensors sketch (ASCD_Nano_Get_DS18B20_Serials.ino) to get your DS serails
DeviceAddress tempSensorSerial[5] =
{
{0x28, 0x55, 0xE9, 0x79, 0x97, 0x05, 0x03, 0x86},
{0x28, 0x90, 0x09, 0x79, 0x97, 0x02, 0x03, 0x46},
{0x28, 0xCF, 0x2B, 0x79, 0x97, 0x05, 0x03, 0x59},
{0x28, 0x4B, 0x6C, 0x79, 0x97, 0x04, 0x03, 0x13},
{0x28, 0xC8, 0x9C, 0x79, 0x97, 0x05, 0x03, 0xC2}
};
