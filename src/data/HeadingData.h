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

    HeadingSource source =
        HeadingSource::NONE;

    double heading = 0.0;

    double rateOfTurn = 0.0;

    uint32_t lastUpdate = 0;
};