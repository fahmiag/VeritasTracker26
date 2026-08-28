#pragma once

#include <Arduino.h>
#include "mcp_can.h"

#include "../data/PositionData.h"
#include "../data/HeadingData.h"

class SCX20CAN
{
public:

    SCX20CAN();

    bool Begin();

    void Update();

    bool GetPosition(PositionData& position);

    bool GetHeading(HeadingData& heading);

private:

    // -------------------------------------------------
    // CAN
    // -------------------------------------------------

    MCP_CAN can_;

    // -------------------------------------------------
    // Parsed data
    // -------------------------------------------------

    PositionData position_;

    HeadingData heading_;

    // -------------------------------------------------
    // PGN processing
    // -------------------------------------------------

    void ProcessMessage(
        uint32_t canId,
        uint8_t len,
        uint8_t* data
    );

    void Parse127250(
        uint8_t len,
        uint8_t* data
    );

    void Parse129026(
        uint8_t len,
        uint8_t* data
    );



    void Process129029(
        uint8_t len,
        uint8_t* data
    );

    // -------------------------------------------------
    // 129029 Fast Packet
    // -------------------------------------------------

    static constexpr uint8_t FAST_PACKET_BUFFER_SIZE = 64;

    struct FastPacketBuffer
    {
        bool active = false;

        uint8_t sequence = 0;
        uint8_t expectedLength = 0;
        uint8_t receivedLength = 0;
        uint8_t nextFrame = 0;

        uint8_t data[FAST_PACKET_BUFFER_SIZE] = {};
    };

    FastPacketBuffer gnssPacket_;

    void ResetFastPacket();

    bool ProcessFastPacket(
        uint8_t len,
        uint8_t* data
    );

    void Parse129029Payload(
        const uint8_t* data,
        uint8_t length
    );

    // -------------------------------------------------
    // Helpers
    // -------------------------------------------------

    uint32_t ExtractPGN(
        uint32_t canId
    );

};