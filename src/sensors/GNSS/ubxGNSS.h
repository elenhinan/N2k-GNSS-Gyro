#pragma once
#include <Arduino.h>
#include <stdint.h>
#include <N2kMessages.h>
#include <Time.h>
#include "ubxCfgMsg.h"
#include "ubxNavMsg.h"
#include "ubxNavPVT.h"
#include "ubxNavSat.h"
#include "ubxNavDop.h"
#include "ubxNavEOE.h"

struct ubxGNSS_settings_t {

};

class uBloxGNSS {
private:
  uint16_t _ubxHeaderBuffer;
  uint8_t _SID;
  uint32_t _last_iTOW;
  uint16_t _rx_classid;
  uint16_t _rx_length;
  uint16_t _rx_checksum;
  uint32_t _current_eoe_time;
  uint32_t _last_LATLONRAPID;
  uint32_t _last_COGSOGRAPID;
  uint32_t _last_GNSS;
  uint32_t _last_SATINFO;
  uint32_t _last_SYSTIME;
  uint32_t _last_DOPDATA;
  uint32_t _last_MAGVAR;
  ubxNavMsg* _rx_NavMsg;
  enum UBXState : uint8_t {
    UBX_RX_CLEANUP,
    UBX_RX_HEADER,
    UBX_RX_CLASSID,
    UBX_RX_PAYLOAD,
    UBX_RX_CHECKSUM,
    UBX_RX_NEWEPOCH
  };
  UBXState _rx_state;
  enum N2KState : uint8_t  {
    N2K_TX_IDLE,
    N2K_TX_START,
    N2K_TX_GNSS,
    N2K_TX_SATINFO,
    N2K_TX_SYSTIME,
    N2K_TX_DOPDATA,
    N2K_TX_MAGVAR,
    N2K_TX_COGSOGRAPID,
    N2K_TX_LATLONRAPID
  };
  N2KState _n2k_state;
  ubxNavSat _packet_ubxNavSat;
  ubxNavPVT _packet_ubxNavPVT;
  ubxNavDOP _packet_ubxNavDOP;
  ubxNavEOE _packet_ubxNavEOE;

  HardwareSerial *_ubxStream;
  Stream *_debugStream;
  uint32_t _serialActivityMillis;
  uint32_t _serialActivityTimeout;
  int8_t _N2kSatInfoCount;
  double _secondsSinceMidnight;
  uint16_t _daysSince1970;
  double _decimalYear;
  double _declination;

  const uint32_t _interval_LATLONRAPID = 200;
  const uint32_t _interval_COGSOGRAPID = 200;
  const uint32_t _interval_GNSS = 1000;
  const uint32_t _interval_SATINFO = 5000;
  const uint32_t _interval_SYSTIME = 1000;
  const uint32_t _interval_DOPDATA = 5000;
  const uint32_t _interval_MAGVAR = 60000;

private:
  static unsigned char _UbxToN2kSvStatus(uint32_t flags);
  void _ChecksumCompute(uint16_t &checksum, uint8_t* data, uint16_t n);
  bool _Read(uint8_t &data);
  bool _Read(uint16_t &data);
  bool _Read(uint32_t &data);
  bool _Read(int32_t &data);
  bool _Read(uint8_t *data, uint16_t bytes);
  bool _Skip(uint16_t bytes);
  bool _FindHeader();
  void _CalculateTime();
  //void _CalculateVariation();
  bool _CreateN2kSatInfo(tN2kMsg &N2kMsg);
  bool _CreateN2kGNSS(tN2kMsg &N2kMsg);
  bool _CreateN2kCOGSOGRapid(tN2kMsg &N2kMsg);
  bool _CreateN2kLatLonRapid(tN2kMsg &N2kMsg);
  bool _CreateN2kSystemTime(tN2kMsg &N2kMsg);
  bool _CreateN2kGNSSDOPData(tN2kMsg &N2kMsg);
  bool _CreateN2kMagneticVariation(tN2kMsg &N2kMsg);

public:
  uBloxGNSS();
  void Setup();
  void SetUbxStream(HardwareSerial* stream) { _ubxStream=stream; }
  void SetDebugStream(Stream* stream) { _debugStream=stream; }
  void ParseMessages();
  bool N2kMsgAvailable();
  void N2kMsgGet(tN2kMsg &N2kMsg);
  double latitude() { return (double)_packet_ubxNavPVT.lat * 1e-7; }
  double longitude() { return (double)_packet_ubxNavPVT.lon * 1e-7; }
  double altitude() { return (double)_packet_ubxNavPVT.hMSL * 1e-3; }
  double declination() { return _declination; }
  double decYear() { return _decimalYear; }
};