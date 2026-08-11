#pragma once

#include <Arduino.h>

enum class HeadingSource
{
    NONE,
    SCX20
};

struct HeadingData
{
    bool valid = false;

    HeadingSource source = HeadingSource::NONE;

    double heading = 0.0;     // degrees, 0-360
    double rateOfTurn = 0.0; // degrees/sec

    uint32_t lastUpdate = 0;
};