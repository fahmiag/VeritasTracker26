#pragma once

#include <Arduino.h>
#include <driver/twai.h>

#include "../data/PositionData.h"
#include "../data/HeadingData.h"

class SCX20CAN
{
public:

    bool Begin();

    void Update();

    bool GetPosition(PositionData& position);

    bool GetHeading(HeadingData& heading);

private:

    PositionData position_;

    HeadingData heading_;

    void ProcessMessage(const twai_message_t& message);

    void ParseVesselHeading(const twai_message_t& message);

    void ParsePositionRapidUpdate(const twai_message_t& message);

    void ParseCOGSOG(const twai_message_t& message);

    uint32_t ExtractPGN(uint32_t canId);
};