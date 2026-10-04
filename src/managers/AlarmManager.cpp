#include "AlarmManager.h"

#include "../config/PinMap.h"

bool AlarmManager::Read(const Input& input)
{
    return (digitalRead(input.pin) == LOW) == input.activeLow;
}

void AlarmManager::Begin()
{
    if (initialized_)
    {
        return;
    }

    emergency_.pin = PinMap::BUTTON_PANIC;
    emergency_.activeLow = true;

    tamper_.pin = PinMap::BUTTON_ENCLOSURE;
    tamper_.activeLow = TAMPER_ACTIVE_LOW;

    // GPIO39 requires an external pull-up.
    pinMode(emergency_.pin, INPUT);

    pinMode(tamper_.pin, INPUT_PULLUP);

    const uint32_t now = millis();

    emergency_.candidate = Read(emergency_);
    tamper_.candidate = Read(tamper_);

    emergency_.changedAt = now;
    tamper_.changedAt = now;

    // stable starts false, so an active input at startup
    // also generates an event after debounce.
    initialized_ = true;
}

bool AlarmManager::Sample(
    Input& input,
    uint32_t now)
{
    const bool value = Read(input);

    if (value != input.candidate)
    {
        input.candidate = value;
        input.changedAt = now;
    }

    if (input.stable != input.candidate &&
        uint32_t(now - input.changedAt) >= DEBOUNCE_MS)
    {
        input.stable = input.candidate;

        // Return true only on activation.
        return input.stable;
    }

    return false;
}

void AlarmManager::Update(
    const TelemetryPacket& navigation)
{
    if (!initialized_)
    {
        return;
    }

    const uint32_t now = millis();

    const bool emergency =
        Sample(emergency_, now);

    const bool tamper =
        Sample(tamper_, now) && tamperArmed_;

    // Set both latches before taking event snapshots.
    if (emergency)
    {
        emergencyLatched_ = true;
    }

    if (tamper)
    {
        tamperLatched_ = true;
    }

    if (emergency)
    {
        Queue(0, navigation);
    }

    if (tamper)
    {
        Queue(1, navigation);
    }
}

uint8_t AlarmManager::ApplyStatus(
    uint8_t status) const
{
    status &= static_cast<uint8_t>(
        ~(Telemetry::EMERGENCY | Telemetry::TAMPER)
    );

    if (emergencyLatched_)
    {
        status |= Telemetry::EMERGENCY;
    }

    if (tamperLatched_)
    {
        status |= Telemetry::TAMPER;
    }

    return status;
}

uint8_t AlarmManager::Index(
    Telemetry::Type type)
{
    return type == Telemetry::Type::EMERGENCY ? 0 : 1;
}

void AlarmManager::Queue(
    uint8_t index,
    const TelemetryPacket& navigation)
{
    Pending& event = pending_[index];

    // Includes an event currently being transmitted.
    if (event.pending)
    {
        return;
    }

    event = Pending{};

    // Capture navigation now, rather than at retry time.
    event.packet = navigation;

    event.packet.type =
        index == 0
            ? Telemetry::Type::EMERGENCY
            : Telemetry::Type::TAMPER;

    event.packet.status =
        ApplyStatus(event.packet.status);

    event.pending = true;
}

bool AlarmManager::GetNext(
    TelemetryPacket& packet,
    bool& firstAttempt) const
{
    if (!initialized_ || activeIndex_ >= 0)
    {
        return false;
    }

    const uint32_t now = millis();

    // Emergency is checked first.
    for (uint8_t i = 0; i < 2; ++i)
    {
        const Pending& event = pending_[i];

        if (!event.pending)
        {
            continue;
        }

        if (event.retryWaiting &&
            uint32_t(now - event.failedAt) < RETRY_MS)
        {
            continue;
        }

        packet = event.packet;

        firstAttempt =
            !event.sequenceAssigned;

        return true;
    }

    return false;
}

void AlarmManager::Queued(
    const TelemetryPacket& packet)
{
    const uint8_t index =
        Index(packet.type);

    pending_[index].packet.sequence =
        packet.sequence;

    pending_[index].sequenceAssigned = true;
    pending_[index].retryWaiting = false;

    activeIndex_ =
        static_cast<int8_t>(index);
}

void AlarmManager::QueueFailed(
    Telemetry::Type type)
{
    Pending& event =
        pending_[Index(type)];

    event.retryWaiting = true;
    event.failedAt = millis();
}

void AlarmManager::Complete(
    bool success)
{
    if (activeIndex_ < 0)
    {
        return;
    }

    Pending& event =
        pending_[activeIndex_];

    if (success)
    {
        event.pending = false;
    }
    else
    {
        QueueFailed(event.packet.type);
    }

    activeIndex_ = -1;

    // Successful transmission does not clear the alarm latch.
}

bool AlarmManager::AcknowledgeEmergency()
{
    if (!initialized_ ||
        emergency_.stable ||
        Read(emergency_) ||
        pending_[0].pending)
    {
        return false;
    }

    emergencyLatched_ = false;

    return true;
}

bool AlarmManager::AcknowledgeTamper()
{
    if (!initialized_ ||
        tamper_.stable ||
        Read(tamper_) ||
        pending_[1].pending)
    {
        return false;
    }

    tamperLatched_ = false;

    return true;
}

void AlarmManager::SetTamperArmed(
    bool armed,
    const TelemetryPacket& navigation)
{
    const bool rearmed =
        armed && !tamperArmed_;

    tamperArmed_ = armed;

    // Rearming an already-open enclosure raises an alarm.
    if (initialized_ &&
        rearmed &&
        tamper_.stable)
    {
        tamperLatched_ = true;

        Queue(1, navigation);
    }
}