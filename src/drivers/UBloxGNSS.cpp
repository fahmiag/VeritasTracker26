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

    return true;
}


void UBloxGNSS::Update()
{
    while (serial_.available())
    {
        char c = serial_.read();

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
}


bool UBloxGNSS::GetPosition(PositionData& position)
{
    position = position_;

    return position_.valid;
}