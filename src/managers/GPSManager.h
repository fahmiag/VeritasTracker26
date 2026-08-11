#pragma once

#include "../drivers/UBloxGNSS.h"
#include "../data/PositionData.h"

class GPSManager
{
public:

    bool Begin();

    void Update();

    bool GetPosition(PositionData& position);

private:

    UBloxGNSS ublox_;

    PositionData position_;
};