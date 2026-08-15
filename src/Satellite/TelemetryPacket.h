#pragma once

#include <stdint.h>

struct TelemetryPacket
{
    uint8_t version;
    uint8_t flags;

    int32_t latitude;
    int32_t longitude;

    uint16_t heading;

    uint8_t satellites;

    uint16_t sequence;
};

