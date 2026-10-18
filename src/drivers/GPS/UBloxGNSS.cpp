#include "UBloxGNSS.h"

#include "../../config/PinMap.h"


UBloxGNSS::UBloxGNSS()
    : serial_(2)
{
}


bool UBloxGNSS::Begin()
{
    if (uartStarted_)
    {
        serial_.end();
    }

    serial_.begin(
        BAUD_RATE,
        SERIAL_8N1,
        PinMap::UBLOX_RX,
        PinMap::UBLOX_TX
    );

    uartStarted_ = true;
    // Clear old parser and position data on every restart.
    gps_ = TinyGPSPlus{};

    position_ = PositionData{}; //Create NEW instance of PositionData name position_, 
                                // Value initialized object, set all to init value/state (default)
                                //use for reset/clear the structure

    position_.source = PositionSource::UBLOX;

    // Serial.print("Position Source");
    // switch (position_.source)
    // {
    //     case PositionSource::NONE:
    //         Serial.println("NONE");
    //         break;

    //     case PositionSource::UBLOX:
    //         Serial.println("UBLOX");
    //         break;

    //     case PositionSource::SCX20:
    //         Serial.println("SCX20");
    //         break;

    //     default:
    //         Serial.println("UNKNOWN");
    //         break;
    // }


    return true;

    //TODO: add return false. HOW?
    //Check if serial string available, but this will block the code
}


void UBloxGNSS::Update()
{
if (!uartStarted_)
{
    return;
}

    while (serial_.available())
    {
        const char c = static_cast<char>(serial_.read());

        // encode() returns true when a valid NMEA sentence completes.
        if (gps_.encode(c))
        {
            lastValidSentence_ = millis();
        }
    }

    if (uint32_t(millis() - lastValidSentence_) >= INIT_RETRY_MS)
    {
        Serial.println(
            "[UBLOX] No valid NMEA for 60 seconds; restarting UART"
        );

        Begin();
        return;
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

//position  - function parameter
//position_ - UBloxGNSS class member

bool UBloxGNSS::GetPosition(PositionData& position)
{
    position = position_;

    return position_.valid;
}
