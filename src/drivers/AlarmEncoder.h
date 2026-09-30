#pragma once
#include "AlarmManager.h"
#include <stddef.h>

// Proposed NEW alarm payload; does not replace your position encoder.
class AlarmEncoder {
public:
    static constexpr size_t Size = 12;
    // Wire: version(1), type(1), id LE(4), occurrence uptime LE(4),
    // latched flags(1), reserved=0(1). No implicit struct serialization.
    static size_t Encode(const AlarmManager::Event& event,
                         uint8_t version, uint8_t emergencyType, uint8_t tamperType,
                         uint8_t* output, size_t capacity);
};
