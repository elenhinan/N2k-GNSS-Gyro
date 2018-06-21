#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <i2c_t3.h>
#include <U8g2lib.h>
#include <NMEA2000_CAN.h>  // This will automatically choose right CAN library and create suitable NMEA2000 object
#include <N2kMessages.h>
#include <ActisenseReader.h>
//#include <Time.h>
#include "sensors/GNSS/ubxGNSS.h"
//#include "sensors/Gyro/bnoGyro.h"
#include "pins.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
//#include "imu.h"

#define ActisenseUART Serial     // Programming port?
#define ActisenseBaud 115200
#define UbloxUART     Serial1    // PCB port
#define UbloxBaud     115200
#define DebugUART     Serial
#define DebugBaud     115200

//#define UbxDirect
// List here messages your device will transmit.

uint32_t last_update;

const unsigned long TransmitMessages[] PROGMEM={130310L,130311L,130312L,0};

tActisenseReader ActisenseReader;

void update_display(double heading, double roll, double pitch, bool valid);

void HandleStreamN2kMsg(const tN2kMsg &N2kMsg);

//void PassThruMode();

// void SetN2kPGN129029();
// void SetN2kPGN129025();
// void SetN2kPGN129026();
// void SetN2kPGN129540();
// void SetN2kPGN129539();
// void SetN2kPGN126992();
// void SetN2kPGN127258();

/*
129029  GNSS Position Data        1 Time/Second
129539  GNSS DOPs                 1 Time/Second
129025  Position, Rapid Update    5 Times/Second
129026  COG and SOG, Rapid Update 5 Times/Second
129540  GNSS Satellites in View   1 Time/Second

126992  System Time               1 Time/Second
127258  Magnetic Variation        1 Time/Second
*/
