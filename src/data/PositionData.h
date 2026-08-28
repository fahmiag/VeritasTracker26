#pragma once

#include <stdint.h>

enum class PositionSource : uint8_t
{
    NONE,
    UBLOX,
    SCX20
};


struct PositionData
{
    bool valid = false;

    PositionSource source = PositionSource::UBLOX;

    // UTC date/time from GPS
    uint16_t year = 0;
    uint8_t month = 0;
    uint8_t day = 0;

    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;

    //UNIX Which Source provide direct timestamp
    uint32_t timestamp = 0;

    // Position
    double latitude = 0.0;
    double longitude = 0.0;

    // Navigation
    double speed = 0.0;
    double course = 0.0;

    // GPS quality
    uint8_t satellites = 0;
    double hdop = 0.0;

    // millis() timestamp of last received update
    uint32_t lastUpdate = 0;
};