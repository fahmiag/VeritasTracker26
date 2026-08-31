#pragma once

#include <stdint.h>
#include <stddef.h>

#include "../data/TelemetryPacket.h"

class TelemetryEncoder
{
public:

    static constexpr size_t PACKET_SIZE = 22;

    static bool Encode(
        const TelemetryPacket& packet,
        uint8_t* buffer,
        size_t bufferSize
    );
};