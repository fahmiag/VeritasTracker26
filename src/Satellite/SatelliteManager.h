#pragma once

#include <Arduino.h>

#include "TelemetryPacket.h"
#include "TelemetryEncoder.h"
#include "../drivers/RockBlockPlus9704.h"

class SatelliteManager
{
public:

    SatelliteManager(
        RockBlockPlus9704& modem
    );

    bool Begin();

    void Update();

    bool SendTelemetry(
        const TelemetryPacket& packet
    );

    bool IsBusy() const;

private:

    RockBlockPlus9704& modem_;

    uint16_t sequence_;

    static constexpr uint16_t TELEMETRY_TOPIC = 1;

    char payloadHex_[128];
};