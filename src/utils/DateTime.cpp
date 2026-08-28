#include "DateTime.h"

UTCDateTime EpochToUTC(
    uint32_t daysSince1970,
    double utcTimeSeconds
)
{
    UTCDateTime result{};

    // -------------------------------------------------
    // Convert days since 1970-01-01 to calendar date
    // -------------------------------------------------

    uint32_t days = daysSince1970;

    uint16_t year = 1970;

    while (true)
    {
        bool leapYear =
            ((year % 4 == 0 && year % 100 != 0) ||
             (year % 400 == 0));

        uint16_t daysInYear =
            leapYear ? 366 : 365;

        if (days < daysInYear)
        {
            break;
        }

        days -= daysInYear;
        year++;
    }

    const uint8_t daysInMonth[] =
    {
        31, 28, 31, 30,
        31, 30, 31, 31,
        30, 31, 30, 31
    };

    uint8_t month = 1;

    while (month <= 12)
    {
        uint8_t monthDays =
            daysInMonth[month - 1];

        // February in leap year
        if (month == 2)
        {
            bool leapYear =
                ((year % 4 == 0 && year % 100 != 0) ||
                 (year % 400 == 0));

            if (leapYear)
            {
                monthDays = 29;
            }
        }

        if (days < monthDays)
        {
            break;
        }

        days -= monthDays;
        month++;
    }

    result.year = year;
    result.month = month;
    result.day = days + 1;

    // -------------------------------------------------
    // Convert time of day
    // -------------------------------------------------

    uint32_t totalSeconds =
        static_cast<uint32_t>(utcTimeSeconds);

    result.hour =
        totalSeconds / 3600;

    totalSeconds %= 3600;

    result.minute =
        totalSeconds / 60;

    result.second =
        totalSeconds % 60;

    return result;
}


uint32_t UTCToEpoch(
    uint16_t year,
    uint8_t month,
    uint8_t day,
    uint8_t hour,
    uint8_t minute,
    uint8_t second)
{
    uint32_t days = 0;

    // Days from 1970 up to the beginning of this year
    for (uint16_t y = 1970; y < year; y++)
    {
        days +=
            ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0))
            ? 366
            : 365;
    }

    // Days in each month
    const uint8_t daysInMonth[] =
    {
        31, 28, 31, 30,
        31, 30, 31, 31,
        30, 31, 30, 31
    };

    for (uint8_t m = 1; m < month; m++)
    {
        days += daysInMonth[m - 1];

        // Leap year
        if (m == 2 &&
            ((year % 4 == 0 && year % 100 != 0) ||
             (year % 400 == 0)))
        {
            days++;
        }
    }

    days += day - 1;

    return
        days * 86400UL +
        hour * 3600UL +
        minute * 60UL +
        second;
}