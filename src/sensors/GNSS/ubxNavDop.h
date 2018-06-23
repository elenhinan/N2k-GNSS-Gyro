#pragma once
#include "ubxNavMsg.h"

struct ubxNavDOP : ubxNavMsg
{
    static const uint16_t class_id = 0x0401;
    ubxNavDOP() : ubxNavMsg(ubxNavDOP::class_id) {};
//  format      name            unit        description
    uint16_t    gDOP;       //  -           Geometric DOP
    uint16_t    pDOP;       //  -           Position DOP
    uint16_t    tDOP;       //  -           Time DOP
    uint16_t    vDOP;       //  -           Vertical DOP
    uint16_t    hDOP;       //  -           Horizontal DOP
    uint16_t    nDOP;       //  -           Northing DOP
    uint16_t    eDOP;       //  -           Easting DOP
};