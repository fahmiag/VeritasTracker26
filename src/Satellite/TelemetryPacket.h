#pragma once

#include <stdint.h>

struct TelemetryPacket
{
    uint8_t version;
    uint8_t type;
    uint8_t flags;

    uint32_t timestamp;

    int32_t latitude;
    int32_t longitude;

    uint16_t cog;
    uint16_t heading;
};