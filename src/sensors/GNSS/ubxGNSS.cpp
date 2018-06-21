#include "ubxGNSS.h"

uBloxGNSS::uBloxGNSS()
{
  _debugStream = NULL;
  _ubxHeaderBuffer = 0x0000;
  _sid = 0;
};

void uBloxGNSS::Setup()
{
  //ubxCfgMsg cfgMsg;
  //cfgMsg.rate = 1;
  //cfgMsg.msgClassId = ubxNavSat::class_id;
}

void uBloxGNSS::_ChecksumCompute(uint8_t* data, uint16_t n) {
  for(uint16_t i=0;i<n;i++)
  {
    ((uint8_t*)&_Checksum)[0] += data[i];
    ((uint8_t*)&_Checksum)[1] += ((uint8_t*)&_Checksum)[0];
  }
};

bool uBloxGNSS::_ChecksumCompare(uint16_t checksum) {
  return _Checksum == checksum;
};

bool uBloxGNSS::_Read(uint8_t &data) {
  uint8_t n = _ubxStream->readBytes((uint8_t*)&data,sizeof(data));
  _ChecksumCompute((uint8_t*)&data,(uint16_t)sizeof(data));
  return n == sizeof(data);
};
bool uBloxGNSS::_Read(uint16_t &data) {
  uint8_t n = _ubxStream->readBytes((uint8_t*)&data,sizeof(data));
  _ChecksumCompute((uint8_t*)&data,(uint16_t)sizeof(data));
  return n == sizeof(data);
};
bool uBloxGNSS::_Read(uint32_t &data) {
  uint8_t n = _ubxStream->readBytes((uint8_t*)&data,sizeof(data));
  _ChecksumCompute((uint8_t*)&data,(uint16_t)sizeof(data));
  return n == sizeof(data);
};

bool uBloxGNSS::_Read(int32_t &data) {
  uint8_t n = _ubxStream->readBytes((uint8_t*)&data,sizeof(data));
  _ChecksumCompute((uint8_t*)&data,(uint16_t)sizeof(data));
  return n == sizeof(data);
};

bool uBloxGNSS::_Read(uint8_t *data, uint16_t bytes) {
  int n = _ubxStream->readBytes(data, bytes);
  _ChecksumCompute(data, bytes);
  return n == bytes;  
}

bool uBloxGNSS::_Skip(uint16_t bytes) {
  uint8_t *buf = new uint8_t[bytes];
  int n = _ubxStream->readBytes(buf, bytes);
  _ChecksumCompute(buf, bytes);
  delete [] buf;
  return n == bytes;
}

bool uBloxGNSS::_FindHeader() {
  bool header = false;
  while(_ubxStream->available() && !header) {
    _ubxHeaderBuffer = _ubxHeaderBuffer<<8 | _ubxStream->read();
    header = (_ubxHeaderBuffer == ubxNavMsg::header);
  }
  return header;
};

unsigned char uBloxGNSS::_UbxToN2kSvStatus(uint32_t flags) {
  unsigned char status;
  if((flags & ubxSFqualityInd) < 4) // if less than 4 then satelite not tracked (?)
    status = N2kSFNotTracked;
  else if(flags & ubxSFsvUsed)
    status = N2kSFUsed;
  else
    status = N2kSFTracked;
  if(flags & ubxSFdiffCorr)
    status +=3;
  return status;
}

bool uBloxGNSS::ParseMessages(tN2kMsg &N2kMsg) {
  if(!_FindHeader())
    return false;

  _Checksum = 0x0000;
  _classid = 0x0000;
  _ubxLength = 0x0000;
  _Read(_classid);
  _Read(_ubxLength);

  switch(_classid) 
  {
    case ubxNavSat::class_id:
      _Read((uint8_t*)&_packet_ubxNavSat, _ubxLength);
      _debugStream->print("ubxNavSat recieved, itow: ");
      _debugStream->print(_packet_ubxNavSat.iTOW);
      _debugStream->print(", ");
      break;
    case ubxNavPVT::class_id:
      _Read((uint8_t*)&_packet_ubxNavPVT, _ubxLength);
      _debugStream->print("ubxNavPVT recieved, itow: ");
      _debugStream->print(_packet_ubxNavPVT.iTOW);
      _debugStream->print(", ");
      break;
    case ubxNavDOP::class_id:
      _Read((uint8_t*)&_packet_ubxNavDOP, _ubxLength);
      _debugStream->print("ubxNavDOP recieved, itow: ");
      _debugStream->print(_packet_ubxNavDOP.iTOW);
      _debugStream->print(", ");
      break;
    case ubxNavEOE::class_id:
      _debugStream->print("ubxNavEOE recieved, itow: ");
      _Read((uint8_t*)&_packet_ubxNavEOE, _ubxLength);
      _debugStream->print(_packet_ubxNavEOE.iTOW);
      _debugStream->print(", ");
      _sid++;
      break;
    default:
      _Skip(_ubxLength);
  }

  uint16_t checksum;
  _ubxStream->readBytes((uint8_t*)&checksum,2);

  if (_ChecksumCompare(checksum)) {
    if(_debugStream) {
      _debugStream->println("Checksum ok");
    }
  }
  else {
    if(_debugStream) {
      _debugStream->println("Checksum failed");
    }
  }

  if (_packet_ubxNavSat.iTOW == _packet_ubxNavEOE.iTOW)
  {
    if(_debugStream) {
      _debugStream->print("Satelites in use ");
      uint8_t numSvs = _packet_ubxNavSat.numSvs;
      uint8_t usedSvs = 0;
      for(uint8_t i=0; i<numSvs; i++)
      {
        if ((_packet_ubxNavSat.Svs[i].flags & ubxSFsvUsed) != 0)
          usedSvs++;
      }
      _debugStream->print(usedSvs);
      _debugStream->print("/");
      _debugStream->println(numSvs);
    }

    uint8_t numSvs = min(8,_packet_ubxNavSat.numSvs);
    //N2kMsg.SetIsTPMessage();
    SetN2kPGN129540(N2kMsg, _sid, numSvs);
    for(uint8_t i=0; i<numSvs; i++)
    {
      ubxSat_t SV = _packet_ubxNavSat.Svs[i];

      AppendN2kPGN129540(
        N2kMsg,
        SV.svId,
        DegToRad(SV.elev),
        DegToRad(SV.azim),
        (double)SV.cno*1e-1,
        (double)SV.prRes,
        _UbxToN2kSvStatus(SV.flags)
      );
    }
    return true;
  }
  return false;
  if (_packet_ubxNavPVT.iTOW == _packet_ubxNavEOE.iTOW)
  {
    // convert from GNSS time to NMEA2k time
    tmElements_t tm;
    tm.Year = CalendarYrToTm(_packet_ubxNavPVT.year);
    tm.Month = _packet_ubxNavPVT.month;
    tm.Day = _packet_ubxNavPVT.day;
    tm.Hour = 0;
    tm.Minute = 0;
    tm.Second = 0;
    time_t time_midnight = makeTime(tm);
    tm.Hour = _packet_ubxNavPVT.hour;
    tm.Minute = _packet_ubxNavPVT.min;
    tm.Second = _packet_ubxNavPVT.sec;
    time_t gnss_time = makeTime(tm);
    double seconds = (double)(gnss_time-time_midnight) + _packet_ubxNavPVT.nano * 1e-6;
    uint16_t days = time_midnight / SECS_PER_DAY;

    // prepare and send N2kMsg GNSS
    SetN2kGNSS(
      N2kMsg,           // msg
      _sid,                // sid
      days,  // days since 197
      seconds,   // secondssincemidnight
      (double)_packet_ubxNavPVT.lat * 1e-7,  // latitude
      (double)_packet_ubxNavPVT.lon * 1e-7,  // longitude
      (double)_packet_ubxNavPVT.hMSL * 1e-3 , // height over sea level
      N2kGNSSt_GPSSBASWAASGLONASS, // GNSS type
      (_packet_ubxNavPVT.fixType==fix_3d || _packet_ubxNavPVT.fixType==fix_2d)?N2kGNSSm_GNSSfix:N2kGNSSm_noGNSS, // fixtype
      _packet_ubxNavPVT.numSV,
      (double)_packet_ubxNavDOP.hDOP * 1e-2, // HDOP NAV-DOP
      (double)_packet_ubxNavDOP.pDOP * 1e-2, // PDOP,
      (double)(_packet_ubxNavPVT.height - _packet_ubxNavPVT.hMSL) * 1e-3 // geoidal seperation
    );

    // SetN2kCOGSOGRapid(
    //   N2kMsg,
    //   _sid, // sid
    //   N2khr_true,
    //   DegToRad(_packet_ubxNavPVT.headMot * 1e-5),   // course over ground (rad)
    //   _packet_ubxNavPVT.gSpeed * 1e-3              // speed over ground (m/s)
    //   );
    // SetN2kLatLonRapid(
    //   N2kMsg,
    //   (double)_packet_ubxNavPVT.lat * 1e-7,  // latitude
    //   (double)_packet_ubxNavPVT.lon * 1e-7   // longitude
    // );
    return true;
  }
  return false;
};
