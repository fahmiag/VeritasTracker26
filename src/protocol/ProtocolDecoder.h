#ifndef PROTOCOL_DECODER_H
#define PROTOCOL_DECODER_H

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

namespace Protocol
{

// ============================================================
// Protocol constants
// ============================================================

constexpr uint8_t VERSION = 0x01;

// Message types
constexpr uint8_t TYPE_POSITION    = 0x01;
constexpr uint8_t TYPE_ACK         = 0x02;

constexpr uint8_t TYPE_SET_INTERVAL = 0x80;

// Packet sizes
constexpr size_t HEADER_SIZE = 5;
constexpr size_t CRC_SIZE    = 2;

constexpr size_t SET_INTERVAL_PAYLOAD_SIZE = 2;
constexpr size_t SET_INTERVAL_PACKET_SIZE =
    HEADER_SIZE +
    SET_INTERVAL_PAYLOAD_SIZE +
    CRC_SIZE;


// ============================================================
// Decode errors
// ============================================================

enum class DecodeResult : uint8_t
{
    OK = 0,

    NULL_BUFFER,
    TOO_SHORT,

    INVALID_VERSION,
    UNKNOWN_TYPE,

    INVALID_PAYLOAD_LENGTH,
    INVALID_PACKET_LENGTH,

    INVALID_CRC
};


// ============================================================
// Decoded message
// ============================================================

struct DecodedMessage
{
    uint8_t type;
    uint16_t sequence;

    // SET_INTERVAL
    uint16_t intervalMinutes;
};


// ============================================================
// ProtocolDecoder
// ============================================================

class ProtocolDecoder
{
public:

    ProtocolDecoder() = default;

    DecodeResult Decode(
        const uint8_t* data,
        size_t length,
        DecodedMessage& message
    );

    static uint16_t CalculateCrc(
        const uint8_t* data,
        size_t length
    );

    static const char* ResultToString(
        DecodeResult result
    );

private:

    DecodeResult DecodeSetInterval(
        const uint8_t* payload,
        size_t payloadLength,
        DecodedMessage& message
    );

    bool ValidateCrc(
        const uint8_t* data,
        size_t length
    );

    bool IsKnownType(
        uint8_t type
    ) const;
};

} // namespace Protocol

#endif