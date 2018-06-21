#pragma once
#include "ubxNavMsg.h"

struct ubxNavEOE : ubxNavMsg
{
    public:
        static const uint16_t class_id = 0x6101;
};