#pragma once

#include <Arduino.h>

#include "../data/TelemetryPacket.h"
#include "../protocol/TelemetryEncoder.h"
#include "../drivers/Satellite/RockBlockPlus9704.h"
#include "../data/PositionData.h"
#include "../data/HeadingData.h"

#include "managers/ConfigurationManager.h"
#include "../protocol/ProtocolDecoder.h"
#include "../protocol/ProtocolEncoder.h"

#include "../config/Debug.h"

class SatelliteManager
{
public:

    SatelliteManager(
        RockBlockPlus9704& modem,
        ConfigurationManager& configuration
    );

    bool Begin();

    void Update();

    bool SendTelemetry(
        const TelemetryPacket& packet
    );

    bool IsBusy() const; //TODO: Check do I use this anywhere

    void SetTelemetryData( //TODO: Check do I use this anywhere
        const PositionData& position,
        const HeadingData& heading
    );

    // Status
    bool IsReady() const;
    bool IsSending() const;
    bool WasLastSendSuccessful() const;

private:

    RockBlockPlus9704& modem_;
    ConfigurationManager& configuration_;

    Protocol::ProtocolDecoder decoder_;
    uint8_t rxBuffer_[64];

    bool initialized_;

    uint16_t sequence_;

    static constexpr uint16_t TELEMETRY_TOPIC = 1;

    char payloadHex_[128];

    static constexpr uint32_t TELEMETRY_INTERVAL_MS =
        5UL * 60UL * 1000UL;

    uint32_t lastTelemetryTime_;

    bool telemetryTimerStarted_;

        void ProcessIncomingMessage();

    void HandleSetInterval(
        const Protocol::DecodedMessage& message
    );

    bool SendAck(
        uint16_t sequence,
        uint8_t acknowledgedType,
        Protocol::AckResult result,
        Protocol::AckError error
    );

};