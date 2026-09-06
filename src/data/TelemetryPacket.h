#pragma once

#include <stdint.h>
#include "../protocol/TelemetryConfig.h"

struct TelemetryPacket
{
    uint8_t version = Telemetry::VERSION;
    Telemetry::Type type = Telemetry::Type::POSITION;

    // POSITION payload byte 0
    uint8_t status = 0;

    Telemetry::PositionSource positionSource = Telemetry::PositionSource::UBLOX;

    uint16_t sequence = 0;
    uint32_t timestamp = 0;
    int32_t latitude = 0;
    int32_t longitude = 0;
    uint16_t cog = 0;
    uint16_t heading = 0;
};
