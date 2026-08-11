#pragma once

#include "../drivers/UBloxGNSS.h"
#include "../data/PositionData.h"

class GPSManager
{
public:

    enum class GPSMode
    {
        UBLOX_ONLY,
        SCX20_ONLY,
        AUTO
    };

    bool Begin();

    void Update();

    bool GetPosition(PositionData& position);

private:

    UBloxGNSS ublox_;

    PositionData ubloxPosition_;
    PositionData scx20Position_;
    PositionData selectedPosition_;
};