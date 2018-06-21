#include "main.h"

U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, DISP_CS, DISP_DC, DISP_RST);
uBloxGNSS ubxGNSS;
//IMU gyro;
Adafruit_BNO055 bno = Adafruit_BNO055(BNO055_ID, BNO055_ADDRESS_A);

void setup()
{
  ActisenseUART.begin(ActisenseBaud);
  UbloxUART.begin(UbloxBaud);
  DebugUART.begin(DebugBaud);

  ubxGNSS.SetUbxStream(&UbloxUART);
  ubxGNSS.SetDebugStream(&DebugUART);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);

  // Setup SPI
  SPI.setSCK(DISP_SCK);
  SPI.setMOSI(DISP_MOSI);
  SPI.setMISO(DISP_MISO);
  
  // Setup OLED
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.sendBuffer();
  
  // Setup IMU
  pinMode(GYRO_RST, OUTPUT);
  digitalWrite(GYRO_RST, HIGH);
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000L);
  // #define axis_x= 0b00;
  // #define axis_y= 0b01;
  // #define axis_z= 0b10;
  // uint8_t map_x = axis_x;
  // uint8_t map_y = axis_y;
  // uint8_t map_z = axis_z;
  // uint8_t remap = axis_x | axis_y<<2 | axis_z <<4;
  // u8g2.clearBuffer();
  // u8g2.setFont(u8g2_font_ncenB12_tr);
  // u8g2.setCursor(0,20);
  // u8g2.print("I2C:");
  // u8g2.sendBuffer();
  // for(int i=0;i<128;i++)
  // {
  //   Wire.beginTransmission(i);

  //   if(!Wire.endTransmission())
  //   {
  //     u8g2.print(String(i,HEX));
  //     u8g2.print(",");
  //     u8g2.sendBuffer();
  //   }
  // }
  // delay(5000);
  //gyro.setup();
  
  if(!bno.begin()) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.drawStr(0,20,"Gyro Error!");
    u8g2.sendBuffer();
    delay(1000);
  } else {
    sensor_t sensor;
    bno.getSensor(&sensor);
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_profont10_tf);
    u8g2.setCursor(0,10);
    u8g2.print  ("Sensor:       "); u8g2.println(sensor.name);
    u8g2.setCursor(0,20);
    u8g2.print  ("Driver Ver:   "); u8g2.println(sensor.version);
    u8g2.setCursor(0,30);
    u8g2.print  ("Unique ID:    "); u8g2.println(sensor.sensor_id);
    u8g2.setCursor(0,40);
    u8g2.print  ("Resolution:   "); u8g2.print(sensor.resolution); u8g2.println(" xxx");
    u8g2.sendBuffer();
    delay(5000);
    bno.setExtCrystalUse(false);
    bno.setAxisRemap(bno.REMAP_CONFIG_P5);
    bno.setAxisSign(bno.REMAP_SIGN_P5);
    //bno.setAxisRemap(remap);

    last_update = micros();
  }

  // NMEA2000 setup
  //NMEA2000.SetN2kCANSendFrameBufSize(200);
  // Set Product information
  NMEA2000.SetProductInformation("00000001", // Manufacturer's Model serial code
                                 100, // Manufacturer's product code
                                 "GNSS and GYRO",  // Manufacturer's Model ID
                                 "1.0.0.0 (2018-06-19)",  // Manufacturer's Software version code
                                 "1.0.0.0 (2018-06-19)", // Manufacturer's Model version
                                 1 // load equivalent x 50(mA)
                                 );
  // Det device information
  NMEA2000.SetDeviceInformation(31137, // Unique number. Use e.g. Serial number.
                                //140, // Device function=Ownship Attitude (Gyro)). See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                145, // Device function=Ownship Position (GNSS). See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                60, // Device class=Navigation. See codes on  http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                2046 // Just choosen free from code list on http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf
                               );
  

  // If you also want to see all traffic on the bus use N2km_ListenAndNode instead of N2km_NodeOnly below
  NMEA2000.SetMode(tNMEA2000::N2km_ListenAndNode,22); //

  //NMEA2000.ExtendTransmitMessages();
  #ifndef UbxDirect
  NMEA2000.SetForwardStream(&ActisenseUART);
  NMEA2000.SetForwardOwnMessages(true); // Forward own (GPS and Gyro) messages
  NMEA2000.EnableForward(true); // Disable all msg forwarding to USB (=Serial)
  #endif
  // messages we transmit
  //NMEA2000.ExtendTransmitMessages(TransmitMessages);
  NMEA2000.Open();

  ActisenseReader.SetReadStream(&ActisenseUART);
  ActisenseReader.SetMsgHandler(HandleStreamN2kMsg); 
}

void HandleStreamN2kMsg(const tN2kMsg &N2kMsg) {
  // N2kMsg.Print(&Serial);
  NMEA2000.SendMsg(N2kMsg,-1);
}

void loop()
{
  #ifdef UbxDirect
    while(DebugUART.available()) {
      UbloxUART.write(DebugUART.read());
    }
    while(UbloxUART.available()) {
      DebugUART.write(UbloxUART.read());
    }
    return;
  #endif

  uint32_t temp_time = micros();
  if(temp_time >= last_update + 100000)
  {
    last_update = temp_time;
    imu::Vector<3> euler = bno.getVector(bno.VECTOR_EULER);
    double heading = euler.x();
    double roll = euler.y();
    double pitch = euler.z();  
    bool valid = bno.isFullyCalibrated();
   
    update_display(heading, roll, pitch, valid);

    // send nmea2k msg
    tN2kMsg N2kMsg;
    SetN2kAttitude(N2kMsg, 1, 0, DegToRad(pitch), DegToRad(roll));
    NMEA2000.SendMsg(N2kMsg);
    SetN2kMagneticHeading(N2kMsg, 1, DegToRad(heading), N2kDoubleNA, N2kDoubleNA); // deviation - get from ardupilot code
    NMEA2000.SendMsg(N2kMsg);
  }

  NMEA2000.ParseMessages();
  ActisenseReader.ParseMessages();
  
  tN2kMsg N2kMsg;
  if (ubxGNSS.ParseMessages(N2kMsg))
    NMEA2000.SendMsg(N2kMsg);
}

void update_display(double heading, double roll, double pitch, bool valid)
{
  digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN)==0);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_profont10_tf);
  u8g2.setCursor(0,8);
  u8g2.print("pitch:   ");
  u8g2.print(pitch, 1);
  u8g2.print("\xB0");
  u8g2.setCursor(0,16);
  u8g2.print("roll:    ");
  u8g2.print(roll, 1);
  u8g2.print("\xB0");
  u8g2.setCursor(0,24);
  u8g2.print("heading: ");
  u8g2.print(heading, 1);
  u8g2.print("\xB0");
  u8g2.setCursor(0,32);
  u8g2.print("valid:   ");
  u8g2.print(valid);
  u8g2.setCursor(0,40);
  double lat = ubxGNSS.latitude();
  char ns = lat>0?'N':'S';
  double lon = ubxGNSS.longitude();
  char ew = lon>0?'E':'W';
  u8g2.print("lat:     ");
  u8g2.print(abs(lat), 5);
  u8g2.print("\xB0 ");;u8g2.print(ns);
  u8g2.setCursor(0,48);
  u8g2.print("lon:     ");
  u8g2.print(abs(lon), 5);
  u8g2.print("\xB0 ");u8g2.print(ew);
  u8g2.setCursor(0,56);
  u8g2.sendBuffer();
}

void sendstuff()
{
  //SetN2kSystemTime (126992)
  //
  // GGA,GLL and GSA -> 126992. 129025, 129029, 129033, 129539
  // GSV -> 129540
  // HDG -> 127250
  // ROT -> 127251
  // RMC -Z 126992,127250,127258,129025,129026,129029,129033
  // VTG -> 129026
  // Vessel Heading
  // Angles should be in radians
  //SetN2kPGN127250
  // Rate of turn
  // Angles should be in radians
  //SetN2kPGN127251
  // GNSS Position Data
  //SetN2kPGN129029
  // https://github.com/tkurki/navgauge/wiki/Global-Navigation-Satellite-System-(gnss)-Object ?
}
/*
void PassThruMode()
{
  while(ubxSerial.available())
  {
    Terminal.write(ubxSerial.read());
  }
  while(Terminal.available())
  {
    ubxSerial.write(Terminal.read());
  }
}
*/