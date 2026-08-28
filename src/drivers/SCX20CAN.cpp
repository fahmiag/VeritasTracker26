#include "SCX20CAN.h"

#include "../config/PinMap.h"


// =====================================================
// Constructor
// =====================================================

SCX20CAN::SCX20CAN()
    : can_(PinMap::SCX20_CAN_CS)
{
    gnssPacket_ = {};
}


// =====================================================
// Begin
// =====================================================

bool SCX20CAN::Begin()
{
    // -------------------------------------------------
    // Clear parsed data
    // -------------------------------------------------

    position_ = PositionData{};
    position_.source = PositionSource::SCX20;

    heading_ = HeadingData{};
    heading_.source = HeadingSource::SCX20;

    ResetFastPacket();


    // -------------------------------------------------
    // SPI
    // -------------------------------------------------

    SPI.begin(
        PinMap::SCX20_CAN_SCK,
        PinMap::SCX20_CAN_MISO,
        PinMap::SCX20_CAN_MOSI,
        PinMap::SCX20_CAN_CS
    );


    // -------------------------------------------------
    // MCP2515
    // -------------------------------------------------

    while (CAN_OK != can_.begin(CAN_500KBPS))
    {
        Serial.println(
            "[SCX20] CAN BUS init failed"
        );

        delay(100);
    }

    Serial.println(
        "[SCX20] CAN BUS initialized"
    );

    return true;
}


// =====================================================
// Update
// =====================================================

void SCX20CAN::Update()
{
    uint8_t len = 0;
    uint8_t buf[8];

    /*
     * IMPORTANT:
     *
     * Use while instead of if.
     *
     * This drains all frames currently waiting
     * in the MCP2515 receive buffer.
     */

    while (CAN_MSGAVAIL == can_.checkReceive())
    {
        can_.readMsgBuf(
            &len,
            buf
        );

        uint32_t canId =
            can_.getCanId();

        ProcessMessage(
            canId,
            len,
            buf
        );
    }
}


// =====================================================
// Extract PGN
// =====================================================

uint32_t SCX20CAN::ExtractPGN(
    uint32_t canId
)
{
    uint8_t dataPage =
        (canId >> 24) & 0x01;

    uint8_t pduFormat =
        (canId >> 16) & 0xFF;

    uint8_t pduSpecific =
        (canId >> 8) & 0xFF;


    uint32_t pgn;


    if (pduFormat < 240)
    {
        // PDU1
        pgn =
            ((uint32_t)dataPage << 16) |
            ((uint32_t)pduFormat << 8);
    }
    else
    {
        // PDU2
        pgn =
            ((uint32_t)dataPage << 16) |
            ((uint32_t)pduFormat << 8) |
            pduSpecific;
    }

    return pgn;
}


// =====================================================
// Process CAN message
// =====================================================

void SCX20CAN::ProcessMessage(
    uint32_t canId,
    uint8_t len,
    uint8_t* data
)
{
    uint32_t pgn =
        ExtractPGN(canId);


    switch (pgn)
    {
        case 129029:

            Process129029(
                len,
                data
            );

            break;


        default:
            break;
    }
}


// =====================================================
// Reset Fast Packet
// =====================================================

void SCX20CAN::ResetFastPacket()
{
    gnssPacket_.active = false;

    gnssPacket_.sequence = 0;

    gnssPacket_.expectedLength = 0;

    gnssPacket_.receivedLength = 0;

    gnssPacket_.nextFrame = 0;

    memset(
        gnssPacket_.data,
        0,
        sizeof(gnssPacket_.data)
    );
}


// =====================================================
// Process 129029
// =====================================================

void SCX20CAN::Process129029(
    uint8_t len,
    uint8_t* data
)
{
    if (len != 8)
    {
        return;
    }


    /*
     * Fast Packet header
     *
     * bit 7..5 = sequence ID
     * bit 4..0 = frame number
     */

    uint8_t sequence =
        (data[0] >> 5) & 0x07;

    uint8_t frame =
        data[0] & 0x1F;


    // -------------------------------------------------
    // First frame
    // -------------------------------------------------

    if (frame == 0)
    {
        /*
         * data[1] = total message length
         *
         * For PGN 129029:
         *
         * 0x2B = 43 bytes
         */

        uint8_t messageLength =
            data[1];


        // Validate length

        if (messageLength == 0 ||
            messageLength > FAST_PACKET_BUFFER_SIZE)
        {
            ResetFastPacket();
            return;
        }


        // Start new packet

        gnssPacket_.active = true;

        gnssPacket_.sequence = sequence;

        gnssPacket_.expectedLength =
            messageLength;

        gnssPacket_.receivedLength = 0;

        gnssPacket_.nextFrame = 1;


        /*
         * First frame:
         *
         * byte 0 = header
         * byte 1 = message length
         * byte 2..7 = payload
         *
         * Therefore 6 payload bytes.
         */

        for (uint8_t i = 2; i < 8; i++)
        {
            if (gnssPacket_.receivedLength <
                gnssPacket_.expectedLength)
            {
                gnssPacket_.data[
                    gnssPacket_.receivedLength++
                ] = data[i];
            }
        }


        // -------------------------------------------------
        // Debug
        // -------------------------------------------------

        Serial.print(
            "[SCX20] 129029 START seq="
        );

        Serial.print(
            sequence
        );

        Serial.print(
            " length="
        );

        Serial.println(
            messageLength
        );


        return;
    }


    // -------------------------------------------------
    // Ignore frames if no packet is active
    // -------------------------------------------------

    if (!gnssPacket_.active)
    {
        return;
    }


    // -------------------------------------------------
    // Check sequence
    // -------------------------------------------------

    if (sequence != gnssPacket_.sequence)
    {
        /*
         * A different sequence means that our
         * current packet was interrupted.
         */

        ResetFastPacket();

        return;
    }


    // -------------------------------------------------
    // Check frame number
    // -------------------------------------------------

    if (frame != gnssPacket_.nextFrame)
    {
        /*
         * Missing or out-of-order frame.
         */

        Serial.print(
            "[SCX20] 129029 frame error. Expected "
        );

        Serial.print(
            gnssPacket_.nextFrame
        );

        Serial.print(
            " got "
        );

        Serial.println(
            frame
        );


        ResetFastPacket();

        return;
    }


    // -------------------------------------------------
    // Copy payload
    // -------------------------------------------------

    for (uint8_t i = 1; i < 8; i++)
    {
        if (gnssPacket_.receivedLength <
            gnssPacket_.expectedLength)
        {
            gnssPacket_.data[
                gnssPacket_.receivedLength++
            ] = data[i];
        }
    }


    gnssPacket_.nextFrame++;


    // -------------------------------------------------
    // Packet complete?
    // -------------------------------------------------

    if (gnssPacket_.receivedLength >=
        gnssPacket_.expectedLength)
    {
        Serial.print(
            "[SCX20] 129029 COMPLETE seq="
        );

        Serial.print(
            gnssPacket_.sequence
        );

        Serial.print(
            " length="
        );

        Serial.println(
            gnssPacket_.receivedLength
        );


        /*
         * We now have the complete 43-byte
         * PGN 129029 payload.
         */

        // TODO:
        // Parse129029Payload()


        ResetFastPacket();
    }
}


// =====================================================
// Get Position
// =====================================================

bool SCX20CAN::GetPosition(
    PositionData& position
)
{
    position = position_;

    return position_.valid;
}


// =====================================================
// Get Heading
// =====================================================

bool SCX20CAN::GetHeading(
    HeadingData& heading
)
{
    heading = heading_;

    return heading_.valid;
}