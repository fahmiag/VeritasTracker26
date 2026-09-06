#ifndef PROTOCOL_ENCODER_H
#define PROTOCOL_ENCODER_H

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

#include "ProtocolConfig.h"

namespace Protocol
{

// ------------------------------------------------------------
// ProtocolEncoder
// ------------------------------------------------------------

class ProtocolEncoder
{
public:

    static bool EncodeAck(
        uint16_t sequence,
        uint8_t acknowledgedType,
        AckResult result,
        AckError error,
        uint8_t* buffer,
        size_t bufferSize,
        size_t& encodedLength
    );

    static uint16_t CalculateCrc(
        const uint8_t* data,
        size_t length
    );

private:

    static void WriteUint16BE(
        uint8_t* destination,
        uint16_t value
    );
};

} // namespace Protocol

#endif