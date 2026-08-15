#pragma once

#include <stdint.h>
#include "TelemetryPacket.h"

class TelemetryEncoder
{
public:
    static constexpr uint8_t PACKET_SIZE = 15;

    static uint8_t Encode(
        const TelemetryPacket& packet,
        uint8_t* buffer
    );
};