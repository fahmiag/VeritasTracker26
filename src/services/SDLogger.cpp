#include "SDLogger.h"


SDLogger::SDLogger(SDCard& sdCard)
    : sdCard_(sdCard),
      ready_(false),
      currentDate_{0},
      lastFlushTime_(0)
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


    if (!EnsureLogDirectory())
    {
        Serial.println(
            "SDLogger: failed to create /logs"
        );

        return false;
    }


    ready_ = true;

    Serial.println(
        "SDLogger: ready"
    );

    return true;
}

bool SDLogger::EnsureLogDirectory()
{
    return sdCard_.CreateDirectory(
        "/logs"
    );
}

bool SDLogger::CreateNewFile(
    const char* date)
{
    char filename[40];

    snprintf(
        filename,
        sizeof(filename),
        "/logs/%s.csv",
        date
    );

    Serial.print("SDLogger: opening ");
    Serial.println(filename);

    bool fileExists = sdCard_.Exists(filename);

    logFile_ =
        sdCard_.OpenAppend(filename);


    if (!logFile_)
    {
        Serial.println(
            "SDLogger: failed to open log file"
        );

        return false;
    }

    if (!fileExists)
    {
        Serial.println("SDLogger: new file - writing header");
        WriteHeader();
    }


    strncpy(
        currentDate_,
        date,
        sizeof(currentDate_) - 1
    );

    currentDate_[
        sizeof(currentDate_) - 1
    ] = '\0';


    // ---------------------------------
    // New file?
    // ---------------------------------

    // if (logFile_.size() == 0)
    // {
    //     Serial.println(
    //         "SDLogger: new file - writing header"
    //     );


    //     if (!WriteHeader())
    //     {
    //         Serial.println(
    //             "SDLogger: failed to write header"
    //         );

    //         CloseCurrentFile();

    //         return false;
    //     }
    // }
    // else
    // {
    //     Serial.println(
    //         "SDLogger: existing file - header skipped"
    //     );
    // }


    lastFlushTime_ = millis();


    Serial.print(
        "SDLogger: logging to "
    );

    Serial.println(filename);


    return true;
}

bool SDLogger::WriteHeader()
{
    const char* header =
        "timestamp,"
        "position_source,"
        "latitude,"
        "longitude,"
        "speed_knots,"
        "course,"
        "satellites,"
        "hdop,"
        "heading,"
        "heading_source";


    return sdCard_.WriteLine(
        logFile_,
        header
    );
}

bool SDLogger::IsNewDay(
    const PositionData& position)
{
    char date[11];

    snprintf(
        date,
        sizeof(date),

        "%04d-%02d-%02d",

        position.year,
        position.month,
        position.day
    );


    return strcmp(
        date,
        currentDate_
    ) != 0;
}

bool SDLogger::OpenDailyFile(
    const PositionData& position)
{
    char date[11];


    snprintf(
        date,
        sizeof(date),

        "%04d-%02d-%02d",

        position.year,
        position.month,
        position.day
    );


    if (logFile_)
    {
        if (!IsNewDay(position))
        {
            return true;
        }


        Serial.println(
            "SDLogger: new UTC day"
        );


        Flush();

        CloseCurrentFile();
    }


    return CreateNewFile(date);
}

bool SDLogger::LogNavigation(
    const PositionData& position,
    const HeadingData& heading)
{
    if (!ready_)
        return false;


    if (!position.valid)
        return false;


    if (!OpenDailyFile(position))
        return false;


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


    char line[300];


    snprintf(
        line,
        sizeof(line),

        "%04d-%02d-%02dT"
        "%02d:%02d:%02dZ,"
        "%s,"
        "%.7f,"
        "%.7f,"
        "%.2f,"
        "%.2f,"
        "%u,"
        "%.2f,"
        "%.2f,"
        "%s",

        position.year,
        position.month,
        position.day,

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


    return sdCard_.WriteLine(
        logFile_,
        line
    );
}

void SDLogger::Update()
{
    if (!ready_)
        return;


    uint32_t now = millis();


    if (
        now - lastFlushTime_
        >= FLUSH_INTERVAL_MS
    )
    {
        Flush();

        lastFlushTime_ = now;
    }
}

bool SDLogger::Flush()
{
    if (!logFile_)
        return false;


    return sdCard_.Flush(
        logFile_
    );
}

void SDLogger::CloseCurrentFile()
{
    if (logFile_)
    {
        sdCard_.Flush(logFile_);
        sdCard_.Close(logFile_);

        Serial.println("SDLogger: log file closed");
    }
}