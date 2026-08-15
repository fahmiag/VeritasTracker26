#include "TimeUtils.h"

static bool IsLeapYear(uint16_t year)
{
    return ((year % 4 == 0 && year % 100 != 0) ||
            (year % 400 == 0));
}

uint32_t DateTimeToUnix(
    uint16_t year,
    uint8_t month,
    uint8_t day,
    uint8_t hour,
    uint8_t minute,
    uint8_t second)
{
    uint32_t days = 0;

    for (uint16_t y = 1970; y < year; y++)
    {
        days += IsLeapYear(y) ? 366 : 365;
    }

    static const uint16_t daysBeforeMonth[] =
    {
        0,
        31,
        59,
        90,
        120,
        151,
        181,
        212,
        243,
        273,
        304,
        334
    };

    days += daysBeforeMonth[month - 1];

    if (month > 2 && IsLeapYear(year))
    {
        days++;
    }

    days += day - 1;

    return
        days * 86400UL +
        hour * 3600UL +
        minute * 60UL +
        second;
}