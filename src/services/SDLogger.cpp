#include "SDLogger.h"


SDLogger::SDLogger(SDCard& sdCard)
    : sdCard_(sdCard)
{
}


bool SDLogger::Begin()
{
    if (!sdCard_.IsReady())
    {
        Serial.println(
            "SDLogger: SD card not ready"
        );

        return false;
    }


    if (!HeaderExists())
    {
        return WriteHeader();
    }


    return true;
}


bool SDLogger::HeaderExists()
{
    return sdCard_.Exists(
        logFile_
    );
}


bool SDLogger::WriteHeader()
{
    const char* header =
        "time,"
        "position_source,"
        "latitude,"
        "longitude,"
        "speed_knots,"
        "course,"
        "satellites,"
        "hdop,"
        "heading,"
        "heading_source\n";


    if (!sdCard_.WriteFile(
        logFile_,
        header
    ))
    {
        Serial.println(
            "SDLogger: failed to write header"
        );

        return false;
    }


    Serial.println(
        "SDLogger: CSV header created"
    );

    return true;
}

bool SDLogger::LogPosition(
    const PositionData& position)
{
    if (!sdCard_.IsReady())
    {
        return false;
    }


    char buffer[256];


    const char* source;


    switch (position.source)
    {
        case PositionSource::UBLOX:
            source = "UBLOX";
            break;

        case PositionSource::SCX20:
            source = "SCX20";
            break;

        default:
            source = "NONE";
            break;
    }


    snprintf(
        buffer,
        sizeof(buffer),

        "%02d:%02d:%02d,"
        "%s,"
        "%.7f,"
        "%.7f,"
        "%.2f,"
        "%.2f,"
        "%u,"
        "%.2f,"
        ","
        "\n",

        position.hour,
        position.minute,
        position.second,

        source,

        position.latitude,
        position.longitude,

        position.speed,
        position.course,

        position.satellites,

        position.hdop
    );


    return sdCard_.AppendFile(
        logFile_,
        buffer
    );
}

bool SDLogger::LogHeading(
    const HeadingData& heading)
{
    if (!sdCard_.IsReady())
    {
        return false;
    }


    char buffer[128];


    const char* source;


    switch (heading.source)
    {
        case HeadingSource::SCX20:
            source = "SCX20";
            break;

        default:
            source = "NONE";
            break;
    }


    snprintf(
        buffer,
        sizeof(buffer),

        ","
        ","
        ","
        ","
        ","
        ","
        ","
        ","
        "%.2f,"
        "%s\n",

        heading.heading,

        source
    );


    return sdCard_.AppendFile(
        logFile_,
        buffer
    );
}

bool SDLogger::LogNavigation(
    const PositionData& position,
    const HeadingData& heading)
{
    if (!sdCard_.IsReady())
    {
        return false;
    }


    char buffer[300];


    const char* positionSource;

    switch (position.source)
    {
        case PositionSource::UBLOX:
            positionSource = "UBLOX";
            break;

        case PositionSource::SCX20:
            positionSource = "SCX20";
            break;

        default:
            positionSource = "NONE";
            break;
    }


    const char* headingSource;

    switch (heading.source)
    {
        case HeadingSource::SCX20:
            headingSource = "SCX20";
            break;

        default:
            headingSource = "NONE";
            break;
    }


    snprintf(
        buffer,
        sizeof(buffer),

        "%02d:%02d:%02d,"
        "%s,"
        "%.7f,"
        "%.7f,"
        "%.2f,"
        "%.2f,"
        "%u,"
        "%.2f,"
        "%.2f,"
        "%s\n",

        position.hour,
        position.minute,
        position.second,

        positionSource,

        position.latitude,
        position.longitude,

        position.speed,
        position.course,

        position.satellites,

        position.hdop,

        heading.heading,

        headingSource
    );


    return sdCard_.AppendFile(
        logFile_,
        buffer
    );
}