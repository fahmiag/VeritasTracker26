#include "UBloxGNSS.h"

#include "../config/PinMap.h"


UBloxGNSS::UBloxGNSS()
    : serial_(2)
{
}


bool UBloxGNSS::Begin()
{
    serial_.begin(
        BAUD_RATE,
        SERIAL_8N1,
        PinMap::UBLOX_RX,
        PinMap::UBLOX_TX
    );

    position_ = PositionData{};

    position_.source = PositionSource::UBLOX;

    Serial.print("Position Source");
    switch (position_.source)
    {
        case PositionSource::NONE:
            Serial.println("NONE");
            break;

        case PositionSource::UBLOX:
            Serial.println("UBLOX");
            break;

        case PositionSource::SCX20:
            Serial.println("SCX20");
            break;

        default:
            Serial.println("UNKNOWN");
            break;
    }


    return true;
}


void UBloxGNSS::Update()
{
    while (serial_.available())
    {
        char c = serial_.read();
       // Serial.print(c);

        gps_.encode(c);
    }


    // ---------------------------------------------
    // Location
    // ---------------------------------------------

    if (gps_.location.isValid() &&
        gps_.location.isUpdated())
    {
        position_.latitude =
            gps_.location.lat();

        position_.longitude =
            gps_.location.lng();

        position_.valid = true;

        position_.source =
            PositionSource::UBLOX;

        position_.lastUpdate =
            millis();
    }


    // ---------------------------------------------
    // Speed
    // ---------------------------------------------

    if (gps_.speed.isUpdated())
    {
        position_.speed =
            gps_.speed.knots();
    }


    // ---------------------------------------------
    // Course
    // ---------------------------------------------

    if (gps_.course.isUpdated())
    {
        position_.course =
            gps_.course.deg();
    }


    // ---------------------------------------------
    // Satellites
    // ---------------------------------------------

    if (gps_.satellites.isValid())
    {
        position_.satellites =
            gps_.satellites.value();
    }


    // ---------------------------------------------
    // HDOP
    // ---------------------------------------------

    if (gps_.hdop.isValid())
    {
        position_.hdop =
            gps_.hdop.hdop();
    }


    // ---------------------------------------------
    // UTC time
    // ---------------------------------------------

    if (gps_.time.isValid())
    {
        position_.hour =
            gps_.time.hour();

        position_.minute =
            gps_.time.minute();

        position_.second =
            gps_.time.second();
    }

    // ---------------------------------------------
    // UTC Date
    // ---------------------------------------------

    if (gps_.date.isValid())
    {
        position_.year =
            gps_.date.year();

        position_.month =
            gps_.date.month();

        position_.day =
            gps_.date.day();
    }

    position_.timestamp =
    UTCToEpoch(
        position_.year,
        position_.month,
        position_.day,
        position_.hour,
        position_.minute,
        position_.second
    );


}


bool UBloxGNSS::GetPosition(PositionData& position)
{
    position = position_;

    return position_.valid;
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