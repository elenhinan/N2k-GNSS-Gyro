#pragma once
#include <Arduino.h>
#include <stdint.h>

struct ubxCfg {
        static const uint16_t header = 0x62B5;
        static const uint16_t class_id = 0x0106;
};

struct ubxCfgMsg : ubxCfg {
    public:
        static const uint16_t header = 0x62B5;
        static const uint16_t class_id = 0x0106;
        uint16_t msgClassId;    //  classid for message to configure
        uint8_t  rate;          // rate, send per n navigation solution
};