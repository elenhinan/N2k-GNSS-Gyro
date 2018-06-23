#pragma once
#include <Arduino.h>
#include <stdint.h>

struct ubxNavMsg {
    protected:
        static uint32_t* lastEOEiTOW;
        bool checksumOk;    
    public: // functions
        ubxNavMsg(uint32_t id) : class_id(id) {};
        void setChecksum(bool ok) { checksumOk = ok; }
        bool isChecksumOk() { return checksumOk; }
        bool isCurrent() { return iTOW == *lastEOEiTOW; }
        bool isValid() { return isChecksumOk() && isCurrent(); }
        uint8_t* dataptr() { return (uint8_t*)&iTOW;}
    public: // members
        static const uint16_t header = 0xB562;
        const uint16_t class_id;
    
        // only payload from here:        
        uint32_t iTOW;       //  ms          GPS time of week of the navigation epoch
};