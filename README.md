# N2k-GNSS-Gyro
Arduino based NMEA2000 Gyrocompass, Attitude and GNSS/GPS running on teensy 3.2


Working PGN's:
* PGN127258 - SatInfo
* PGN129029 - GNSS Position etc.
* PGN129026 - COGSOGRapid
* PGN129025 - LatLonRapid
* PGN126992 - SystemTime
* PGN129539 - GNSSDOPData
* PGN127258 - MagneticVariation
Sort of working PGN's but missing calibration:
* PGN127257 - Attitude
* PGN127250 - MagneticHeading / TrueHeading (with WMM2015 modeling)


Hardware needed:

* uBlox Neo-M8n gnss device connected via serial1:
Setup: disable NMEA on uart1, enable NAV TX
Enable only these packages:
    - UBX-NAV-DOP
    - UBX-NAV-PVT
    - UBX-NAV-SAT
    - UBX-NAV-EOE
Set sampling rate to 200 ms, navigation ratio to 1 for 5 Hz refresh rate.

* BNO055 IMU as Gyro, connected via I2C

* SSD1306 OLED connected via SPI (so far)

* SN65HVD230 CAN-bus interface


Added WMM2015 magnetic variation calculation based on code from NOAA (legacy c code, restructured for arduino)
Uses python script (utils/MagDevGenerator.py) to read coefficents-file (utils/WMM2015/WMM.cof) which can be 
downloaded from NOAA. Precalculations are then performed and it generates c++ code for the tables needed for
model calculations and outputs into "WMM2016cof.h"

Results from arduino code is equal to WMM2015 test-values
output:
    Testing WMM-2015 model
    i: 0 should be: -3.850 got: -3.849
    i: 1 should be: 0.570 got: 0.570
    i: 2 should be: 69.810 got: 69.805
    i: 3 should be: -4.270 got: -4.271
    i: 4 should be: 0.560 got: 0.556
    i: 5 should be: 69.810 got: 69.219
    i: 6 should be: -2.750 got: -2.751
    i: 7 should be: 0.320 got: 0.322
    i: 8 should be: 69.580 got: 69.585
    i: 9 should be: -3.170 got: -3.171
    i: 10 should be: 0.320 got: 0.319
    i: 11 should be: 69.000 got: 69.004

Pinouts in "pins.h"

Use an external 3.3 power, as the teensy cannot power all devices (>100 mA)

Huge thanks to the creator of the NMEA2000 arduino library (https://github.com/ttlappalainen/NMEA2000)