#pragma once

#include <Arduino.h>

struct UTCDateTime
{
    uint16_t year;
    uint8_t month;
    uint8_t day;

    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

UTCDateTime EpochToUTC(
    uint32_t daysSince1970,
    double utcTimeSeconds
);

uint32_t UTCToEpoch(
    uint16_t year,
    uint8_t month,
    uint8_t day,
    uint8_t hour,
    uint8_t minute,
    uint8_t second);