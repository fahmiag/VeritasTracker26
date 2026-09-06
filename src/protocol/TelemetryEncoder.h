#pragma once

#include <stdint.h>
#include <stddef.h>
#include "../data/TelemetryPacket.h"

class TelemetryEncoder
{
public:
    static constexpr size_t PACKET_SIZE = Telemetry::POSITION_PACKET_SIZE;

    static bool Encode(
        const TelemetryPacket& packet,
        uint8_t* buffer,
        size_t bufferSize
    );

    static uint16_t Crc16(
        const uint8_t* data,
        size_t length
    );
};
