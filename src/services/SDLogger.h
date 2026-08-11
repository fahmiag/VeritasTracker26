#pragma once

#include <Arduino.h>

#include "../drivers/SDCard.h"
#include "../data/PositionData.h"
#include "../data/HeadingData.h"

class SDLogger
{
public:

    explicit SDLogger(SDCard& sdCard);

    bool Begin();

    bool LogPosition(
        const PositionData& position
    );

    bool LogHeading(
        const HeadingData& heading
    );

    bool LogNavigation(
        const PositionData& position,
        const HeadingData& heading
    );

private:

    SDCard& sdCard_;

    const char* logFile_ =
        "/navigation.csv";

    bool WriteHeader();

    bool HeaderExists();
};