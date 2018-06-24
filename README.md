# N2k-GNSS-Gyro
Arduino based NMEA2000 Gyrocompass, Attitude and GNSS/GPS running on teensy 3.2

Still beta.

Stuff needed:

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

Pinouts in "pins.h"

Use an external 3.3 power, as the teensy cannot power all devices (>100 mA)

Huge thanks to the creator of the NMEA2000 arduino library (https://github.com/ttlappalainen/NMEA2000)