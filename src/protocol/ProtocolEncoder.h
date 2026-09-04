#ifndef PROTOCOL_ENCODER_H
#define PROTOCOL_ENCODER_H

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

namespace Protocol
{

constexpr uint8_t VERSION = 0x01;

constexpr uint8_t TYPE_POSITION     = 0x01;
constexpr uint8_t TYPE_ACK          = 0x02;
constexpr uint8_t TYPE_SET_INTERVAL = 0x80;

constexpr size_t HEADER_SIZE = 5;
constexpr size_t CRC_SIZE = 2;

constexpr size_t ACK_PAYLOAD_SIZE = 3;

constexpr size_t ACK_PACKET_SIZE =
    HEADER_SIZE +
    ACK_PAYLOAD_SIZE +
    CRC_SIZE;


// ------------------------------------------------------------
// ACK result
// ------------------------------------------------------------

enum class AckResult : uint8_t
{
    SUCCESS = 0x00,
    FAILED  = 0x01
};


// ------------------------------------------------------------
// ACK error
// ------------------------------------------------------------

enum class AckError : uint8_t
{
    NONE = 0x00,

    INVALID_COMMAND       = 0x01,
    INVALID_VALUE         = 0x02,
    UNSUPPORTED_VERSION   = 0x03,
    INVALID_CRC           = 0x04,
    INTERNAL_ERROR        = 0x05
};


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