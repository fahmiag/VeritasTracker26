#pragma once

#include <Arduino.h>

#include "../data/TelemetryPacket.h"

class AlarmManager
{
public:
    void Begin();

    // Poll inputs and capture navigation when an alarm activates.
    void Update(const TelemetryPacket& navigation);

    // Add latched alarm states to the navigation status byte.
    uint8_t ApplyStatus(uint8_t status) const;

    // Returns the highest-priority event eligible for transmission.
    bool GetNext(
        TelemetryPacket& packet,
        bool& firstAttempt
    ) const;

    // Called after SatelliteManager accepts an alarm transmission.
    void Queued(const TelemetryPacket& packet);

    // Called when the modem rejects the initial queue request.
    void QueueFailed(Telemetry::Type type);

    // Called when the queued transmission finishes.
    void Complete(bool success);

    bool EmergencyActive() const
    {
        return emergency_.stable;
    }

    bool EnclosureOpen() const
    {
        return tamper_.stable;
    }

    bool EmergencyLatched() const
    {
        return emergencyLatched_;
    }

    bool TamperLatched() const
    {
        return tamperLatched_;
    }

    bool AcknowledgeEmergency();
    bool AcknowledgeTamper();

    void SetTamperArmed(
        bool armed,
        const TelemetryPacket& navigation
    );

private:
    struct Input
    {
        uint8_t pin = 0;
        bool activeLow = true;

        bool candidate = false;
        bool stable = false;

        uint32_t changedAt = 0;
    };

    struct Pending
    {
        TelemetryPacket packet{};

        bool pending = false;
        bool sequenceAssigned = false;
        bool retryWaiting = false;

        uint32_t failedAt = 0;
    };

    static constexpr uint32_t DEBOUNCE_MS = 50;
    static constexpr uint32_t RETRY_MS = 30000;

    // Closed enclosure contact = LOW.
    // Open enclosure contact = HIGH.
    //
    // Change to true if your switch produces LOW when opened.
    static constexpr bool TAMPER_ACTIVE_LOW = false;

    static bool Read(const Input& input);

    static bool Sample(
        Input& input,
        uint32_t now
    );

    static uint8_t Index(Telemetry::Type type);

    void Queue(
        uint8_t index,
        const TelemetryPacket& navigation
    );

    Input emergency_{};
    Input tamper_{};

    // Index 0 = emergency; index 1 = tamper.
    Pending pending_[2]{};

    int8_t activeIndex_ = -1;

    bool initialized_ = false;

    bool emergencyLatched_ = false;
    bool tamperLatched_ = false;
    bool tamperArmed_ = true;
};