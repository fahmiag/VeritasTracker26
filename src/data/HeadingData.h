#pragma once

#include <stdint.h>

enum class HeadingSource : uint8_t
{
    NONE = 0,
    SCX20
};

struct HeadingData
{
    bool valid = false;

    HeadingSource source = HeadingSource::NONE;

    // Heading in degrees
    double heading = 0.0;

    // Rate of turn in degrees/second
    double rateOfTurn = 0.0;

    // millis() timestamp of last received update
    uint32_t lastUpdate = 0;
};