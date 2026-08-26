#pragma once

#include <Arduino.h>
#include <SPI.h>
#include "../lib/MCP_CAN/mcp_can.h"

#include "../data/PositionData.h"
#include "../data/HeadingData.h"

class SCX20CAN
{
public:

    SCX20CAN();

    bool Begin();

    void Update();

    bool GetPosition(
        PositionData& position
    );

    bool GetHeading(
        HeadingData& heading
    );

private:

    static constexpr uint8_t CS_PIN = 5;

    MCP_CAN can_;

    PositionData position_;

    HeadingData heading_;

    void ProcessMessage(
        uint32_t canId,
        uint8_t* data,
        uint8_t length
    );

    void ParseVesselHeading(
        uint8_t* data,
        uint8_t length
    );

    void ParsePositionRapidUpdate(
        uint8_t* data,
        uint8_t length
    );

    void ParseCOGSOG(
        uint8_t* data,
        uint8_t length
    );

    uint32_t ExtractPGN(
        uint32_t canId
    );
};