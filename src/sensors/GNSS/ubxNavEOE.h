#pragma once
#include "ubxNavMsg.h"

struct ubxNavEOE : ubxNavMsg
{
    static const uint16_t class_id = 0x6101;
    ubxNavEOE() : ubxNavMsg(ubxNavEOE::class_id) {
        ubxNavMsg::lastEOEiTOW = &iTOW;
    }
};