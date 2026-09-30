#include "AlarmManager.h"

AlarmManager::AlarmManager(InputConfig emergency, InputConfig tamper,
                           uint32_t retryMs)
    : emergency_{emergency}, tamper_{tamper}, retryMs_(retryMs) {}

bool AlarmManager::Input::Read() const {
    return (digitalRead(config.pin) == LOW) == config.activeLow;
}
void AlarmManager::Input::Begin(uint32_t now) {
    pinMode(config.pin, config.mode);
    candidate = Read();
    stable = false; // An active input at startup is debounced then reported.
    changedAt = now;
}
bool AlarmManager::Input::Sample(uint32_t now) {
    const bool value = Read();
    if (value != candidate) { candidate = value; changedAt = now; }
    if (candidate != stable && uint32_t(now - changedAt) >= config.debounceMs) {
        stable = candidate;
        return true;
    }
    return false;
}
void AlarmManager::Begin(bool tamperArmed) {
    if (initialized_) return;
    tamperArmed_ = tamperArmed;
    const uint32_t now = millis();
    emergency_.Begin(now);
    tamper_.Begin(now);
    initialized_ = true;
}
uint8_t AlarmManager::Status() const {
    return (emergencyLatched_ ? 1u : 0u) | (tamperLatched_ ? 2u : 0u);
}
void AlarmManager::Raise(Kind kind, uint32_t now) {
    if (kind == Kind::Emergency) emergencyLatched_ = true;
    else tamperLatched_ = true;
    Slot& slot = slots_[static_cast<uint8_t>(kind)];
    // Coalesce repeat activations while this kind is pending or in flight.
    if (slot.pending || (inFlight_ && active_.kind == kind)) return;
    slot.event = Event{kind, nextId_++, now, Status()};
    slot.pending = true;
}
void AlarmManager::Update() {
    if (!initialized_) return;
    const uint32_t now = millis();
    if (emergency_.Sample(now) && emergency_.stable) Raise(Kind::Emergency, now);
    if (tamper_.Sample(now) && tamper_.stable && tamperArmed_) Raise(Kind::Tamper, now);
}
bool AlarmManager::EmergencyActive() const { return emergency_.stable; }
bool AlarmManager::EnclosureOpen() const { return tamper_.stable; }
void AlarmManager::SetTamperArmed(bool armed) {
    const bool rearming = armed && !tamperArmed_;
    tamperArmed_ = armed;
    if (initialized_ && rearming && tamper_.stable) Raise(Kind::Tamper, millis());
}
bool AlarmManager::ClearEmergency() {
    if (!initialized_ || EmergencyActive() || slots_[0].pending ||
        (inFlight_ && active_.kind == Kind::Emergency)) return false;
    emergencyLatched_ = false;
    return true;
}
bool AlarmManager::ClearTamper() {
    if (!initialized_ || EnclosureOpen() || slots_[1].pending ||
        (inFlight_ && active_.kind == Kind::Tamper)) return false;
    tamperLatched_ = false;
    return true;
}
uint8_t AlarmManager::ApplyPositionFlags(uint8_t flags, uint8_t emergencyMask,
                                        uint8_t tamperMask) const {
    flags &= static_cast<uint8_t>(~(emergencyMask | tamperMask));
    if (emergencyLatched_) flags |= emergencyMask;
    if (tamperLatched_) flags |= tamperMask;
    return flags;
}
bool AlarmManager::HasPending() const { return slots_[0].pending || slots_[1].pending; }
bool AlarmManager::TakeNext(Event& event) {
    if (!initialized_ || inFlight_) return false;
    const uint32_t now = millis();
    for (auto& slot : slots_) {
        if (!slot.pending) continue;
        if (slot.retryWaiting && uint32_t(now - slot.failedAt) < retryMs_) continue;
        active_ = slot.event;
        slot.pending = false;
        inFlight_ = true;
        slot.retryWaiting = false;
        event = active_;
        return true;
    }
    return false;
}
bool AlarmManager::Complete(uint32_t id, bool success) {
    if (!inFlight_ || id != active_.id) return false;
    if (!success) {
        Slot& slot = slots_[static_cast<uint8_t>(active_.kind)];
        slot.event = active_;
        slot.pending = true;
        slot.failedAt = millis();
        slot.retryWaiting = true;
    }
    inFlight_ = false;
    return true;
}
