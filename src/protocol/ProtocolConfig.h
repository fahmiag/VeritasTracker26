#pragma once

#include <stddef.h>
#include <stdint.h>

namespace Protocol
{
    constexpr uint8_t VERSION = 0x01;

    constexpr uint8_t TYPE_POSITION     = 0x01;
    constexpr uint8_t TYPE_ACK          = 0x02;
    constexpr uint8_t TYPE_EMERGENCY = 0x03;
    constexpr uint8_t TYPE_TAMPER    = 0x04;
    constexpr uint8_t TYPE_POWER_LOSS = 0x05;
    constexpr uint8_t TYPE_SET_INTERVAL = 0x80;

    constexpr size_t HEADER_SIZE = 5;
    constexpr size_t CRC_SIZE = 2;

    constexpr size_t SET_INTERVAL_PAYLOAD_SIZE = 2;
    constexpr size_t SET_INTERVAL_PACKET_SIZE =
        HEADER_SIZE + SET_INTERVAL_PAYLOAD_SIZE + CRC_SIZE;

    constexpr size_t ACK_PAYLOAD_SIZE = 3;
    constexpr size_t ACK_PACKET_SIZE =
        HEADER_SIZE + ACK_PAYLOAD_SIZE + CRC_SIZE;

    enum class AckResult : uint8_t
    {
        SUCCESS = 0x00,
        FAILED  = 0x01
    };

    enum class AckError : uint8_t
    {
        NONE = 0x00,
        INVALID_COMMAND     = 0x01,
        INVALID_VALUE       = 0x02,
        UNSUPPORTED_VERSION = 0x03,
        INVALID_CRC         = 0x04,
        INTERNAL_ERROR      = 0x05
    };
}
