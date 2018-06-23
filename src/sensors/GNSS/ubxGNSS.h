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

#define dim(x) (sizeof(x) / sizeof((x)[0]))

class uBloxGNSS {
private:
  uint8_t _SID;
  uint32_t _last_iTOW;
  uint16_t _ubxHeaderBuffer;
  uint16_t _rx_classid;
  uint16_t _rx_length;
  uint16_t _rx_checksum;
  ubxNavMsg* _rx_NavMsg;
  enum UBXState {
    UBX_RX_HEADER,
    UBX_RX_CLASSID,
    UBX_RX_PAYLOAD,
    UBX_RX_CHECKSUM,
    UBX_RX_NEWEPOCH,
    UBX_RX_CLEANUP
  };
  UBXState _rx_state;
  enum N2KState {
    N2K_TX_IDLE,
    N2K_TX_SLOW,
    N2K_TX_GNSS,
    N2K_TX_SATINFO,
    N2K_TX_RAPID,
    N2K_TX_COGSOGRAPID,
    N2K_TX_LATLONRAPID
  };
  N2KState _n2k_state;
  ubxNavSat _packet_ubxNavSat;
  ubxNavPVT _packet_ubxNavPVT;
  ubxNavDOP _packet_ubxNavDOP;
  ubxNavEOE _packet_ubxNavEOE;
  // ubxNavMsg* const _packets[4] = {
  //                         &_packet_ubxNavSat,
  //                         &_packet_ubxNavPVT,
  //                         &_packet_ubxNavDOP,
  //                         &_packet_ubxNavEOE,
  //                       };
  Stream *_ubxStream;
  Stream *_debugStream;
  bool _newEpoch;
  uint8_t _SlowSID;
  uint8_t _SlowIntervall;
  uint8_t _RapidSID;
  uint8_t _RapidIntervall;

private:
  static unsigned char _UbxToN2kSvStatus(uint32_t flags);
  void _ChecksumCompute(uint8_t* data, uint16_t n);
  bool _Read(uint8_t &data);
  bool _Read(uint16_t &data);
  bool _Read(uint32_t &data);
  bool _Read(int32_t &data);
  bool _Read(uint8_t *data, uint16_t bytes);
  bool _Skip(uint16_t bytes);
  bool _FindHeader();
  void _ReadNavPvt();
  void _ReadNavSat();
  bool _CreateN2kSatInfo(tN2kMsg &N2kMsg);
  bool _CreateN2kGNSS(tN2kMsg &N2kMsg);
  bool _CreateN2kCOGSOGRapid(tN2kMsg &N2kMsg);
  bool _CreateN2kLatLonRapid(tN2kMsg &N2kMsg);

public:
  uBloxGNSS();
  void Setup();
  void SetUbxStream(Stream* stream) { _ubxStream=stream; }
  void SetDebugStream(Stream* stream) { _debugStream=stream; }
  void ParseMessages();
  bool N2kMsgAvailable();
  void N2kMsgGet(tN2kMsg &N2kMsg);
  double latitude() { return (double)_packet_ubxNavPVT.lat * 1e-7; }
  double longitude() { return (double)_packet_ubxNavPVT.lon * 1e-7; }
};