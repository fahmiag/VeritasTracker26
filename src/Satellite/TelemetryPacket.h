#pragma once

#include <stdint.h>
#include "TelemetryConfig.h"

// -----------------------------------------------------------------------------
// Byte 3 - Source field
//
// Bits 0-1 : GPS source
// Bits 2-7 : Reserved
//
// 00 = NONE
// 01 = UBLOX
// 10 = SCX20
// 11 = Reserved
// -----------------------------------------------------------------------------

enum class TelemetryGpsSource : uint8_t
{
    NONE  = 0x00,
    UBLOX = 0x01,
    SCX20 = 0x02
};

enum TelemetryFlags : uint8_t
{
    TELEMETRY_GPS_VALID     = 0x01,
    TELEMETRY_COG_VALID     = 0x02,
    TELEMETRY_HEADING_VALID = 0x04,
    TELEMETRY_TIME_VALID    = 0x08
};

struct TelemetryPacket
{
    uint8_t version = Telemetry::VERSION;

    Telemetry::Type type = Telemetry::Type::POSITION;

    uint8_t flags = 0;

    // Byte 3:
    // bits 0-1 = GPS source
    // bits 2-7 = reserved
    TelemetryGpsSource gpsSource = TelemetryGpsSource::NONE;

    uint16_t sequence = 0;

    uint32_t timestamp = 0;

    int32_t latitude = 0;

    int32_t longitude = 0;

    uint16_t cog = 0;

    uint16_t heading = 0;
};