#pragma once

#include <stdint.h>

namespace Telemetry
{
    constexpr uint8_t VERSION = 1;

    enum class Type : uint8_t
    {
        POSITION = 1
    };

    enum Flags : uint8_t
    {
        NONE = 0x00,

        GPS_VALID     = 0x01,
        HEADING_VALID = 0x02
    };
}