#pragma once

#include <Arduino.h>

enum class PositionSource
{
    NONE,
    UBLOX,
    SCX20
};

struct PositionData
{
    bool valid = false;

    PositionSource source = PositionSource::NONE;

    // UTC date
    uint16_t year = 0;
    uint8_t month = 0;
    uint8_t day = 0;


    // UTC time
    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;

    double latitude = 0.0;
    double longitude = 0.0;

    double speed = 0.0;
    double course = 0.0;

    uint8_t satellites = 0;
    double hdop = 0.0;



    uint32_t lastUpdate = 0;
};