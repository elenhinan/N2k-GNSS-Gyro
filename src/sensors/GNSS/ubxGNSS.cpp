#include "ubxGNSS.h"

uBloxGNSS::uBloxGNSS() :
  _SID(0),
  _last_iTOW(0),
  _ubxHeaderBuffer(0x0000),
  _rx_state(UBX_RX_HEADER),
  _n2k_state(N2K_TX_IDLE),
  _SlowSID(0),
  _SlowIntervall(5),
  _RapidSID(0),
  _RapidIntervall(1)
{
  _debugStream = NULL;
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
    ((uint8_t*)&_rx_checksum)[0] += data[i];
    ((uint8_t*)&_rx_checksum)[1] += ((uint8_t*)&_rx_checksum)[0];
  }
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

void uBloxGNSS::ParseMessages() {
  switch(_rx_state) {
    
    // look for header
    case UBX_RX_HEADER:
      if(_FindHeader())
        _rx_state = UBX_RX_CLASSID;
      else
        break;
    
    // read id and length
    case UBX_RX_CLASSID:
      if(_ubxStream->available() >= (int)sizeof(_rx_classid+_rx_length)) {
        _Read(_rx_classid);
        _Read(_rx_length);
        _rx_state = UBX_RX_PAYLOAD;
      } else
        break;
    
    // read payload
    case UBX_RX_PAYLOAD:
      switch(_rx_classid) {
        case ubxNavSat::class_id:
          _rx_NavMsg = &_packet_ubxNavSat;
          _Read(_rx_NavMsg->dataptr(), _rx_length);
          break;
        case ubxNavPVT::class_id:
          _rx_NavMsg = &_packet_ubxNavPVT;
          _Read(_rx_NavMsg->dataptr(), _rx_length);
          break;
        case ubxNavDOP::class_id:
          _rx_NavMsg = &_packet_ubxNavDOP;
          _Read(_rx_NavMsg->dataptr(), _rx_length);
          break;
        case ubxNavEOE::class_id:
          _rx_NavMsg = &_packet_ubxNavEOE;
          _Read(_rx_NavMsg->dataptr(), _rx_length);
          break;
        default:
          _Skip(_rx_length);
      }
      _rx_state = UBX_RX_CHECKSUM;
    
    // read and compare checksum
    case UBX_RX_CHECKSUM:
      uint16_t ubx_checksum;
      if(_ubxStream->available() >= (int)sizeof(ubx_checksum)) {
        _ubxStream->readBytes((uint8_t*)&ubx_checksum,2);
        _rx_NavMsg->setChecksum(ubx_checksum == _rx_checksum);
        _rx_state = UBX_RX_NEWEPOCH;
      }
      else
        break;

    // set N2K TX state if packets are valid and interval match    
    case UBX_RX_NEWEPOCH:
      if(_packet_ubxNavEOE.iTOW - _last_iTOW > 0 && _packet_ubxNavEOE.isValid()) {
        _last_iTOW = _packet_ubxNavEOE.iTOW;
        _SID++;
        if ((_SID - _RapidSID) >= _RapidIntervall && _packet_ubxNavPVT.isValid()) {
          _RapidSID = _SID;
          _n2k_state = N2K_TX_RAPID;
        }
        if((_SID - _SlowSID) >= _SlowIntervall && _packet_ubxNavPVT.isValid() && _packet_ubxNavSat.isValid() && _packet_ubxNavDOP.isValid()) {
          _SlowSID = _SID;
          _n2k_state = N2K_TX_SLOW;
        }
      }
      _rx_state = UBX_RX_CLEANUP;
    
    // reset variables
    case UBX_RX_CLEANUP:
      _rx_checksum = 0;
      _rx_classid = 0;
      _rx_length = 0;
      _rx_NavMsg = 0;
      _rx_state = UBX_RX_HEADER;

    default:
      break;
  }
}

void uBloxGNSS::N2kMsgGet(tN2kMsg &N2kMsg) {
  switch(_n2k_state) {
    case N2K_TX_SLOW:
    case N2K_TX_GNSS:         _CreateN2kGNSS(N2kMsg); _n2k_state = N2K_TX_SATINFO; break;
    case N2K_TX_SATINFO:      _CreateN2kSatInfo(N2kMsg); _n2k_state = N2K_TX_LATLONRAPID; break;
    case N2K_TX_RAPID:
    case N2K_TX_LATLONRAPID:  _CreateN2kLatLonRapid(N2kMsg); _n2k_state = N2K_TX_COGSOGRAPID; break;
    case N2K_TX_COGSOGRAPID:  _CreateN2kCOGSOGRapid(N2kMsg); _n2k_state = N2K_TX_IDLE; break;
    default: break;
  }
}

bool uBloxGNSS::N2kMsgAvailable() {
  return _n2k_state != N2K_TX_IDLE;
}

bool uBloxGNSS::_CreateN2kSatInfo(tN2kMsg &N2kMsg) {
  if (!_packet_ubxNavSat.isValid())
    return false;
  N2kMsg.Clear();
  uint8_t numSvs = min(16,_packet_ubxNavSat.numSvs);
  //N2kMsg.SetIsTPMessage();
  SetN2kPGN129540(N2kMsg, _SID, numSvs);
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

bool uBloxGNSS::_CreateN2kGNSS(tN2kMsg &N2kMsg) {
  if (!_packet_ubxNavPVT.isValid())
    return false;
  N2kMsg.Clear();
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
    _SID,                // sid
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
  return true;
}

bool uBloxGNSS::_CreateN2kCOGSOGRapid(tN2kMsg &N2kMsg) {
  if(!_packet_ubxNavPVT.isValid())
    return false;
  N2kMsg.Clear();
  SetN2kCOGSOGRapid(
    N2kMsg,
    _SID, // sid
    N2khr_true,
    DegToRad(_packet_ubxNavPVT.headMot * 1e-5),   // course over ground (rad)
    _packet_ubxNavPVT.gSpeed * 1e-3              // speed over ground (m/s)
    );
  return true;
}

bool uBloxGNSS::_CreateN2kLatLonRapid(tN2kMsg &N2kMsg) {
  if(!_packet_ubxNavPVT.isValid())
    return false;
  N2kMsg.Clear();
  SetN2kLatLonRapid(
    N2kMsg,
    (double)_packet_ubxNavPVT.lat * 1e-7,  // latitude
    (double)_packet_ubxNavPVT.lon * 1e-7   // longitude
  );
  return true;
}
