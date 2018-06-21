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

class uBloxGNSS {
private:
  uint8_t _sid;
  uint16_t _ubxHeaderBuffer;
  uint16_t _classid;
  uint16_t _ubxLength;
  uint16_t _Checksum;
  ubxNavSat _packet_ubxNavSat;
  ubxNavPVT _packet_ubxNavPVT;
  ubxNavDOP _packet_ubxNavDOP;
  ubxNavEOE _packet_ubxNavEOE;
  Stream *_ubxStream;
  Stream *_debugStream;

private:
  void _ChecksumCompute(uint8_t* data, uint16_t n);
  void _ChecksumReset();
  bool _ChecksumCompare(uint16_t checksum);
  bool _Read(uint8_t &data);
  bool _Read(uint16_t &data);
  bool _Read(uint32_t &data);
  bool _Read(int32_t &data);
  bool _Read(uint8_t *data, uint16_t bytes);
  bool _Skip(uint16_t bytes);
  bool _FindHeader();
  void _ReadNavPvt();
  void _ReadNavSat();
  static unsigned char _UbxToN2kSvStatus(uint32_t flags);

public:
  uBloxGNSS();
  void Setup();
  void SetUbxStream(Stream* stream) { _ubxStream=stream; }
  void SetDebugStream(Stream* stream) { _debugStream=stream; }
  bool ParseMessages(tN2kMsg &N2kMsg);
  double latitude() { return (double)_packet_ubxNavPVT.lat * 1e-7; }
  double longitude() { return (double)_packet_ubxNavPVT.lon * 1e-7; }
};