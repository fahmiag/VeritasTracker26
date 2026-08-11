#pragma once

#include "../drivers/SCX20CAN.h"
#include "../data/HeadingData.h"

class HeadingManager
{
public:

    bool Begin();

    void Update();

    bool GetHeading(HeadingData& heading);

private:

    SCX20CAN scx20_;

    HeadingData heading_;
};