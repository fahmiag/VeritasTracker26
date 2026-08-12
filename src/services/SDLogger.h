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

    void Update();

    bool IsReady() const;


    bool LogNavigation(
        const PositionData& position,
        const HeadingData& heading
    );
    //===============
    bool LogPosition(
        const PositionData& position
    );

    bool LogHeading(
        const HeadingData& heading
    );


private:

    SDCard& sdCard_;

    File logFile_;

    bool ready_;

    char currentDate_[11];

    uint32_t lastFlushTime_;

    static constexpr uint32_t FLUSH_INTERVAL_MS = 10000;


    bool EnsureLogDirectory();

    bool OpenDailyFile(
        const PositionData& position
    );

    bool CreateNewFile(
        const char* date
    );

    bool WriteHeader();

    bool IsNewDay(
        const PositionData& position
    );

    void CloseCurrentFile();

    bool Flush();
};