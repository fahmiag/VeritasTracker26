#pragma once

#include "../data/PositionData.h"
#include "../data/HeadingData.h"

#include "../drivers/SCX20CAN.h"
#include "../drivers/UBloxGNSS.h"

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
};