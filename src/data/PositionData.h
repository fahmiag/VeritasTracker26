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

    PositionSource source = PositionSource::NONE;

    //TODO: Currently used by ublox and display. Keep this?
    // UTC date/time from GNSS
    uint16_t year = 0;
    uint8_t month = 0;
    uint8_t day = 0;

    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;

    //UTC Unix Epoch timestamp
    //seconds since 1970-01-01 00:00:00 UTC
    uint32_t timestamp = 0;


    // Position
    double latitude = 0.0;
    double longitude = 0.0;
    double altitude = 0.0;

    // Navigation
    double speed = 0.0;
    double course = 0.0;

    // GNSS status
    uint8_t satellites = 0;
    double hdop = 0.0;
    double pdop = 0.0;

    // millis() timestamp of last received update
    uint32_t lastUpdate = 0;
};