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

    const bool previousEmergency = emergency_.stable;
    const bool previousTamper = tamper_.stable;

    const bool emergencyTriggered =
        Sample(emergency_, now);

    const bool tamperTriggered =
        Sample(tamper_, now) && tamperArmed_;

    // Print physical input changes.
    if (emergency_.stable != previousEmergency)
    {
        Serial.println(
            emergency_.stable
                ? "[INPUT] Panic button: PRESSED"
                : "[INPUT] Panic button: RELEASED"
        );
    }

    if (tamper_.stable != previousTamper)
    {
        Serial.println(
            tamper_.stable
                ? "[INPUT] Tamper switch: ENCLOSURE OPEN"
                : "[INPUT] Tamper switch: ENCLOSURE CLOSED"
        );
    }

    // Set both latches before taking event snapshots.
    if (emergencyTriggered)
    {
        emergencyLatched_ = true;
    }

    if (tamperTriggered)
    {
        tamperLatched_ = true;
    }

    if (emergencyTriggered)
    {
        Queue(0, navigation);
    }

    if (tamperTriggered)
    {
        Queue(1, navigation);
    }
}

uint8_t AlarmManager::ApplyStatus(
    uint8_t status) const
{
    status &= static_cast<uint8_t>(
        ~(Telemetry::EMERGENCY |
          Telemetry::TAMPER |
          Telemetry::PSU_POWER)
    );

    if (emergencyLatched_)
    {
        status |= Telemetry::EMERGENCY;
    }

    if (tamperLatched_)
    {
        status |= Telemetry::TAMPER;
    }

    if (powerSource_ == 0)
    {
        status |= Telemetry::PSU_POWER;
    }

    return status;
}

uint8_t AlarmManager::Index(
    Telemetry::Type type)
{
    switch (type)
    {
        case Telemetry::Type::EMERGENCY:
            return 0;

        case Telemetry::Type::TAMPER:
            return 1;

        case Telemetry::Type::POWER_LOSS:
            return 2;

        default:
            return EVENT_COUNT;
    }
}

void AlarmManager::Queue(
    uint8_t index,
    const TelemetryPacket& navigation)
{
    if (index >= EVENT_COUNT)
    {
        return;
    }

    Pending& event = pending_[index];

    // Combine repeated events while one is outstanding.
    if (event.pending)
    {
        return;
    }

    const Telemetry::Type types[EVENT_COUNT] =
    {
        Telemetry::Type::EMERGENCY,
        Telemetry::Type::TAMPER,
        Telemetry::Type::POWER_LOSS
    };

    event = Pending{};

    event.packet = navigation;
    event.packet.type = types[index];

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
    for (uint8_t i = 0; i < EVENT_COUNT; ++i)
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
    const uint8_t index = Index(packet.type);

    if (index >= EVENT_COUNT)
    {
        return;
    }

    pending_[index].packet.sequence = packet.sequence;
    pending_[index].sequenceAssigned = true;
    pending_[index].retryWaiting = false;

    activeIndex_ = static_cast<int8_t>(index);
}

void AlarmManager::QueueFailed(
    Telemetry::Type type)
{
    const uint8_t index = Index(type);

    if (index >= EVENT_COUNT)
    {
        return;
    }

    Pending& event = pending_[index];

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

void AlarmManager::UpdatePowerSource(
    bool onPsu,
    bool onBattery,
    const TelemetryPacket& navigation)
{
    if (!initialized_)
    {
        return;
    }

    // These must describe one selected source.
    if (onPsu && onBattery)
    {
        return;
    }

    const int8_t source =
        onPsu ? 0 : (onBattery ? 1 : 2);

    const uint32_t now = millis();

    // Establish the startup baseline without generating an alert.
    if (powerSource_ < 0)
    {
        powerSource_ = source;
        candidatePowerSource_ = source;
        powerCandidateSince_ = now;

        return;
    }

    if (source != candidatePowerSource_)
    {
        candidatePowerSource_ = source;
        powerCandidateSince_ = now;
    }

    if (candidatePowerSource_ == powerSource_)
    {
        return;
    }

    // Require the new source to remain stable.
    if (uint32_t(now - powerCandidateSince_) <
        POWER_CONFIRM_MS)
    {
        return;
    }

    const bool psuToBattery =
        powerSource_ == 0 &&
        candidatePowerSource_ == 1;

    powerSource_ = candidatePowerSource_;

    if (psuToBattery)
    {
        // Queue() captures navigation and the updated PSU flag.
        Queue(2, navigation);
    }
}