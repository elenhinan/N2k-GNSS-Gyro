#pragma once
#include "ubxNavMsg.h"

#define max_satellites 64

struct ubxSat_t
{
//      format      name                    unit        description
        uint8_t     gnssId;     //          -           gnss identifier
        uint8_t     svId;       //          -           satelite identifier
        uint8_t     cno;        //          dbHz        carrier to noise ratio
        int8_t      elev;       //          deg         elevation
        int16_t     azim;       //          deg         azimuth
        int16_t     prRes;      //          m 1e-1      Pseudo range residual
        uint32_t    flags;      //          -           Bitmask
};

enum ubxSatFlags : uint32_t
{
    ubxSFqualityInd      = 7<<0,
    ubxSFsvUsed          = 1<<3,
    ubxSFhealth          = 3<<4,
    ubxSFdiffCorr        = 1<<6,
    ubxSFsmoothed        = 1<<7,
    ubxSFubxSForbitSource     = 7<<8,
    ubxSFubxSFephAvail        = 1<<11,
    ubxSFubxSFubxSFalmAvail        = 1<<12,
    ubxSFanoAvail        = 1<<13,
    ubxSFubxSFaopAvail        = 1<<14,
    ubxSFsbasCorrUsed    = 1<<16,
    ubxSFubxSFrtcmCorrUsed    = 1<<17,
    ubxSFprCorrUsed      = 1<<20,
    ubxSFcrCorrUsed      = 1<<21,
    ubxSFdoCorrUsed      = 1<<22
};

struct ubxNavSat : ubxNavMsg
{
    public:
        static const uint16_t class_id = 0x3501;
//      format      name                    unit        description
        uint8_t     version;            //  -           Message version (should be 1)
        uint8_t     numSvs;             //  -           Number of satelites
        uint8_t     reserved1[2];
        ubxSat_t    Svs[max_satellites];//  -           Max number of satellites
};