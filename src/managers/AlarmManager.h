#pragma once
#include <Arduino.h>

// Call all methods from App::Run(), including completion handling.
class AlarmManager {
public:
    enum class Kind : uint8_t { Emergency = 0, Tamper = 1 };
    struct InputConfig {
        uint8_t pin;
        bool activeLow;
        uint8_t mode; // INPUT_PULLUP or INPUT, according to wiring
        uint32_t debounceMs;
    };
    struct Event {
        Kind kind;
        uint32_t id;
        uint32_t uptimeMs; // occurrence time, NOT Unix time
        uint8_t status;    // bit 0 emergency latched, bit 1 tamper latched
    };
    AlarmManager(InputConfig emergency, InputConfig tamper,
                 uint32_t retryMs = 30000);
    void Begin(bool tamperArmed = true);
    void Update();
    bool EmergencyActive() const;
    bool EnclosureOpen() const;
    bool EmergencyLatched() const { return emergencyLatched_; }
    bool TamperLatched() const { return tamperLatched_; }
    bool TamperArmed() const { return tamperArmed_; }
    void SetTamperArmed(bool armed);
    // Acknowledge locally or by an authenticated command; refuse while active.
    bool ClearEmergency();
    bool ClearTamper();
    // Supply existing protocol's two FREE, non-overlapping flag masks.
    uint8_t ApplyPositionFlags(uint8_t flags, uint8_t emergencyMask,
                               uint8_t tamperMask) const;
    // Reserve the highest priority pending event. false = no eligible event.
    // The caller MUST subsequently call Complete(id, success).
    bool TakeNext(Event& event);
    bool Complete(uint32_t id, bool success);
    bool Busy() const { return inFlight_; }
    bool HasPending() const;

private:
    struct Input {
        explicit Input(InputConfig value) : config(value) {}
        InputConfig config;
        bool candidate = false;
        bool stable = false;
        uint32_t changedAt = 0;
        bool Sample(uint32_t now);
        bool Read() const;
        void Begin(uint32_t now);
    };
    struct Slot {
        bool pending = false;
        Event event{};
        bool retryWaiting = false;
        uint32_t failedAt = 0;
    };
    void Raise(Kind kind, uint32_t now);
    uint8_t Status() const;
    Input emergency_;
    Input tamper_;
    Slot slots_[2]{};
    Event active_{};
    uint32_t nextId_ = 1;
    uint32_t retryMs_;
    bool inFlight_ = false;
    bool initialized_ = false;
    bool emergencyLatched_ = false;
    bool tamperLatched_ = false;
    bool tamperArmed_ = true;
};
