#pragma once

#include <stdint.h>
#include <stddef.h>

namespace Telemetry
{
    constexpr uint8_t VERSION = 1;

    //the MSB indicate message direction
    //0x8_ means MT.
    enum class Type : uint8_t
    {
        POSITION     = 0x01,
        ACK          = 0x02,
        EMERGENCY    = 0x03,
        TAMPER       = 0x04,
        SET_INTERVAL = 0x80
    };

    // POSITION status byte
    // TODO Remove Unneeded flag
    constexpr uint8_t POSITION_VALID = 0x01;
    constexpr uint8_t COG_VALID      = 0x02;
    constexpr uint8_t HEADING_VALID  = 0x04;
    constexpr uint8_t TIME_VALID     = 0x08;
    constexpr uint8_t SOURCE_SCX20   = 0x10;

    constexpr uint8_t EMERGENCY      = 0x20;
    constexpr uint8_t TAMPER         = 0x40;

    constexpr uint8_t RESERVED_MASK  = 0x80;
    

    enum class PositionSource : uint8_t
    {
        UBLOX = 0,
        SCX20 = 1
    };

    enum class AckResult : uint8_t
    {
        SUCCESS = 0x00,
        FAILED  = 0x01
    };

    enum class AckError : uint8_t
    {
        NONE             = 0x00,
        INVALID_LENGTH   = 0x01,
        INVALID_VALUE    = 0x02,
        UNSUPPORTED_TYPE = 0x03,
        CRC_ERROR        = 0x04,
        VERSION_ERROR    = 0x05
    };

    constexpr size_t COMMON_HEADER_SIZE = 5;
    constexpr size_t CRC_SIZE = 2;
    constexpr size_t POSITION_PAYLOAD_SIZE = 17;
    constexpr size_t POSITION_PACKET_SIZE =
        COMMON_HEADER_SIZE + POSITION_PAYLOAD_SIZE + CRC_SIZE;
    constexpr size_t SET_INTERVAL_PAYLOAD_SIZE = 2;
    constexpr size_t SET_INTERVAL_PACKET_SIZE =
        COMMON_HEADER_SIZE + SET_INTERVAL_PAYLOAD_SIZE + CRC_SIZE;
    constexpr size_t ACK_PAYLOAD_SIZE = 3;
    constexpr size_t ACK_PACKET_SIZE =
        COMMON_HEADER_SIZE + ACK_PAYLOAD_SIZE + CRC_SIZE;
}
