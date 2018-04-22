#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

/// This is the general base class for implementing Protocol encoders/decoders.
class Protocol {
public:
    virtual bool Decode(const byte* data, int dataLen, JsonObject &target) = 0;
    virtual bool Encode(const JsonObject& source, byte* data, int &dataLen, int maxDataLen) = 0;
};

class KakuAProtocol : public Protocol {
public:
    KakuAProtocol () {}
    
    virtual bool Decode(const byte* data, int dataLen, JsonObject &target) {
        if (dataLen != 4) {
            return false;
        }
        uint32_t addr = (((uint32_t)data[0]) << 18) | (((uint32_t)data[1]) << 10) | (((uint32_t)data[2]) << 2) | (data[3] >> 6);
        uint8_t group = (data[3] >> 5) & 0x01;
        uint8_t onOff = (data[3] >> 4) & 0x01;
        uint8_t unit = data[3] & 0x0f;
        target["address"] = addr;
        target["group"] = group;
        target["action"] = onOff ? "on" : "off";
        target["unit"] = unit;
        return true;
    }

    virtual bool Encode(const JsonObject& source, byte* data, int &dataLen, int maxDataLen) {
        if (maxDataLen < 4) {
            return false;
        }
        uint32_t addr = source["address"] | 0;
        uint8_t group = source["group"] | 0;
        const String& action = source["action"];
        uint8_t unit = source["unit"] | 1;
        uint8_t actionBit = 0;
        if (action == "on") {
            actionBit = 1;
        }
        data[0] = (addr >> 18) & 0xFF;
        data[1] = (addr >> 10) & 0xFF;
        data[2] = (addr >> 2) & 0xFF;
        data[3] = ((addr & 0x03) << 6) | ((group & 0x01) << 5) | ((actionBit & 0x01) << 4) | (unit & 0x0F);
        dataLen = 4;
        return true;
    }
};

// Dumb Arduino IDE pre-processing bug - can't put this in the main source file!
/// Structure used to defined each entry in the protocol table.
typedef struct {
    int typecode;
    const char* name;
    Protocol* protocol;
} ProtocolInfo;
