#pragma once

#include <Arduino.h>

#include "TelemetryPacket.h"
#include "TelemetryEncoder.h"
#include "../drivers/RockBlockPlus9704.h"
#include "../data/PositionData.h"
#include "../data/HeadingData.h"

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

    void SetTelemetryData(
        const PositionData& position,
        const HeadingData& heading
    );

private:

    RockBlockPlus9704& modem_;

    bool initialized_;

    uint16_t sequence_;

    static constexpr uint16_t TELEMETRY_TOPIC = 1;

    char payloadHex_[128];

    static constexpr uint32_t TELEMETRY_INTERVAL_MS =
        5UL * 60UL * 1000UL;

    uint32_t lastTelemetryTime_;

    bool telemetryTimerStarted_;
};