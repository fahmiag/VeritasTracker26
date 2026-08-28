#pragma once

#include <Arduino.h>

enum class HeadingSource : uint8_t
{
    NONE = 0,
    UBLOX   = 1,
    SCX20   = 2
};

struct HeadingData
{
    double heading = 0.0;      // degrees
    double rateOfTurn = 0.0; 

    bool valid = false;

    HeadingSource source =
        HeadingSource::NONE;

    uint32_t lastUpdate = 0;
};