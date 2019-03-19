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
    {0x28, 0x81, 0xCB, 0x79, 0x97, 0x05, 0x03, 0xE5},
    {0x28, 0xAA, 0xF5, 0x6F, 0x1D, 0x13, 0x02, 0xAC},
    {0x28, 0xAA, 0x37, 0x78, 0x1D, 0x13, 0x02, 0xB3},
    {0x28, 0xAA, 0xB7, 0x71, 0x1D, 0x13, 0x02, 0xEB},
    {0x28, 0xAA, 0x2D, 0x6F, 0x1D, 0x13, 0x02, 0xCC}
};
