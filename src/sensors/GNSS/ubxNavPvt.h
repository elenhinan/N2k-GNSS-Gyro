#pragma once
#include "ubxNavMsg.h"

enum fixType_t : uint8_t
{
    no_fix = 0,
    dead_reckoning = 1,
    fix_2d = 2,
    fix_3d = 3,
    gnss_dead_reckoning = 4,
    time_only = 5
};

enum pvt_valid : uint8_t
{
    validDate       = 1<<0,     //  valid UTC Date
    validTime       = 1<<2,     //  valid UTC Time of Day
    fullyResolved   = 1<<3,     //  UTC Time of Day has been fully resolved
    validMag        = 1<<4      //  valid Magnetic declination
};

enum pvt_flags : uint8_t
{
    gnssFixOk       = 1<<0,     // valid fix (i.e within DOP & accuracy masks)
    diffSoln        = 1<<1,     // differential corrections were applied
    psm_off         = 0<<2,     // PSM-state psm not active
    psm_enabled     = 1<<2,     // PSM-state enabled
    psm_acquisition = 2<<2,     // PSM-state acquisition
    psm_tracking    = 3<<2,     // PSM-state tracking
    psm_opt_tracking= 4<<2,     // PSM-state power optimized tracking
    psm_inactive    = 5<<2,     // PSM-state inactive
    headVehValid    = 1<<5,     // heading of vehicle is valid
    carrSoln        = 3<<6      // Carrier phase range solution status
};

enum pvt_flags2 : uint8_t
{
    confirmedAvai   = 1<<5,     // information about UTC Date and Time of Day validity confirmation is available
    confirmedDate   = 1<<6,     // UTC Date validity could be confirmed
    confirmedTime   = 1<<7      // UTC Time of Day could be confirmed
};

struct ubxNavPVT : ubxNavMsg
{
    static const uint16_t class_id = 0x0701;
    ubxNavPVT() : ubxNavMsg(ubxNavPVT::class_id) {};
//      format      name            unit        description
    uint16_t    year;       //  y           Year (UTC)
    uint8_t     month;      //  month       Month, range 1..12 (UTC)
    uint8_t     day;        //  day         Day of month, range 1..31 (UTC)
    uint8_t     hour;       //  h           Hour of day, range 0..23 (UTC)
    uint8_t     min;        //  min         Minute of hour, range 0..59 (UTC)
    uint8_t     sec;        //  s           Seconds of minute, range 0..60 (UTC)
    uint8_t     valid;      //  -           validity flags
    uint32_t    tAcc;       //  ns          time estimate accuracy
    int32_t     nano;       //  ns          fractional second
    fixType_t   fixType;    //  -           GNSSfix type
    uint8_t     flags;      //  -           Fix status flag
    uint8_t     flags2;     //  -           additional flags
    uint8_t     numSV;      //  -           number of satelites used
    int32_t     lon;        // 1e-7 deg     longitude
    int32_t     lat;        // 1e-7 deg     latitude
    int32_t     height;     // mm           height above ellipsoid
    int32_t     hMSL;       // mm           height above sea level
    uint32_t    hAcc;       // mm           horizontal accuracy
    uint32_t    vAcc;       // mm           vertical accuracy
    int32_t     velN;       // mm/s         NED north velocity
    int32_t     velE;       // mm/s         NED east velocity
    int32_t     velD;       // mm/s         NED down velocity
    int32_t     gSpeed;     // mm/s         Ground speed
    int32_t     headMot;    // 1e-5 deg     heading of motion
    uint32_t    sAcc;       // mm/s         speed accuracy esitmate
    uint32_t    headAcc;    // 1e-5 deg     heading accuracy
    uint16_t    pDOP;       // -            Position dop
    uint8_t     reserved1[6];
    int32_t     headVeh;    // 1e-5 deg     heading of vehicle 2D (deg)
    int16_t     magDec;     // 1e-2 deg     Magnetic declination
    uint16_t    magAcc;     // 1e-2 deg     Magnetic declination accuracy
};