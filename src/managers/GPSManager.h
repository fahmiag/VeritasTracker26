#pragma once

#include "../data/PositionData.h"
#include "../data/HeadingData.h"

#include "../drivers/CAN/SCX20CAN.h"
#include "../drivers/GPS/UBloxGNSS.h"

class GPSManager
{
public:

    GPSManager(
        SCX20CAN& scx20,
        UBloxGNSS& ublox
    );

    bool Begin();

    void Update();

    bool GetPosition(
        PositionData& position
    );

    bool GetHeading(
        HeadingData& heading
    );

private:

    SCX20CAN& scx20_;
    UBloxGNSS& ublox_;

    static constexpr uint32_t INIT_RETRY_MS = 60UL * 1000UL;

    bool scx20Initialized_ = false;
    uint32_t lastScx20InitAttempt_ = 0;
};