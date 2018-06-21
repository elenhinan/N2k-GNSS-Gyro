#pragma once
#include <Arduino.h>
#include <stdint.h>

struct ubxNavMsg {
    public:
        static const uint16_t header = 0xB562;
        static const uint16_t class_id;
        uint32_t    iTOW;       //  ms          GPS time of week of the navigation epoch
};