#include "SCX20CAN.h"

#include "../../config/PinMap.h"


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
    // MCP2515 Chip Select
    //
    // Shared SPI bus itself is initialized
    // by App::Begin().
    // -------------------------------------------------
    pinMode(
        PinMap::SCX20_CAN_CS,
        OUTPUT
    );

    digitalWrite(
        PinMap::SCX20_CAN_CS,
        HIGH
    );
    
    // -------------------------------------------------
    // Explicitly use global shared SPI bus
    // -------------------------------------------------

    can_.setSPI(&SPI);

    // -------------------------------------------------
    // MCP2515
    // -------------------------------------------------

    //TODO: Test this, using IF and While
    uint8_t attempts = 0;
    while (CAN_OK != can_.begin(CAN_500KBPS) && attempts < 5)
    {
        Serial.print("[SCX20] CAN BUS init failed. Attempts: ");
        Serial.println(attempts);

        attempts++;
        delay(100);

        //return false;
    }

    if (attempts >= 5)
    {
        Serial.println("[SCX20] CAN BUS init failed after retries");
        return false;
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
        // case 129029:
        //     Process129029(len,data);//Multi Frame data
        //     break;
        
        // case 129026:
        //     Parse129026(len,data);//Single frame data
        //     break;
        
        case 127250: //Heading
            Parse127250(len,data); //Single frame data
            break;

        default:
            break;
    }
}

// =====================================================
// Process Dedicate PGN
// =====================================================
//COG SOG
void SCX20CAN::Parse129026(
    uint8_t len,
    uint8_t* data
)
{
    if (len < 8)
    {
        return;
    }


    // Serial.print("COG SOG: ");

    // for (uint8_t i = 0; i < len; i++)
    // {

    //     Serial.print(data[i], HEX);
    //     Serial.print(" ");
    // }

    // Serial.println();


    uint8_t sid =
        data[0];


    // -------------------------------------------------
    // Reference
    // -------------------------------------------------

    uint8_t reference =
        data[1] & 0x03;


    // -------------------------------------------------
    // COG
    // -------------------------------------------------

    uint16_t cogRaw =
        (uint16_t)data[2] |
        ((uint16_t)data[3] << 8);


    // -------------------------------------------------
    // SOG
    // -------------------------------------------------

    uint16_t sogRaw =
        (uint16_t)data[4] |
        ((uint16_t)data[5] << 8);


    // -------------------------------------------------
    // COG
    // -------------------------------------------------

    if (cogRaw != 0xFFFF)
    {
        double cogRadians =
            cogRaw * 0.0001;

        double cogDegrees =
            cogRadians * 180.0 / PI;

        cogDegrees =
            fmod(
                cogDegrees,
                360.0
            );

        if (cogDegrees < 0)
        {
            cogDegrees += 360.0;
        }

        position_.course =
            cogDegrees;
    }


    // -------------------------------------------------
    // SOG
    // -------------------------------------------------

    if (sogRaw != 0xFFFF)
    {
        double speedMetersPerSecond =
            sogRaw * 0.01;

        position_.speed =
            speedMetersPerSecond * 1.943844;
    }


    // -------------------------------------------------
    // Debug
    // -------------------------------------------------

    // Serial.println();

    // Serial.println(
    //     "========== 129026 =========="
    // );

    // Serial.print("SID: ");
    // Serial.println(sid);


    // Serial.print("COG: ");

    // if (cogRaw != 0xFFFF)
    // {
    //     Serial.print(
    //         cogRaw * 0.0001 * 180.0 / PI,
    //         3
    //     );

    //     Serial.println(" deg");
    // }
    // else
    // {
    //     Serial.println("N/A");
    // }


    // Serial.print("SOG: ");

    // if (sogRaw != 0xFFFF)
    // {
    //     Serial.print(
    //         sogRaw * 0.01 * 1.943844,
    //         3
    //     );

    //     Serial.println(" knots");
    // }
    // else
    // {
    //     Serial.println("N/A");
    // }


    // Serial.print("Reference: ");

    // switch (reference)
    // {
    //     case 0:
    //         Serial.println("True");
    //         break;

    //     case 1:
    //         Serial.println("Magnetic");
    //         break;

    //     default:
    //         Serial.println("Reserved");
    //         break;
    // }


    // Serial.println(
    //     "============================"
    // );

    if (cogRaw != 0xFFFF)
    {
        position_.course =
            cogRaw * 0.0001 * 180.0 / PI;
    }

    if (sogRaw != 0xFFFF)
    {
        position_.speed =
            sogRaw * 0.01 * 1.943844;
    }
}

//Heading
void SCX20CAN::Parse127250(
    uint8_t len,
    uint8_t* data
)
{
    if (len < 8)
    {
        return;
    }

    // Serial.print("Heading: ");
    // for (uint8_t i = 0; i < len; i++)
    // {
    //     Serial.print(data[i], HEX);
    //     Serial.print(" ");
    // }
    // Serial.println();

    uint8_t sid = data[0];

    uint16_t headingRaw =
        (uint16_t)data[1] |
        ((uint16_t)data[2] << 8);

    int16_t deviationRaw =
        (int16_t)(
            (uint16_t)data[3] |
            ((uint16_t)data[4] << 8)
        );

    int16_t variationRaw =
        (int16_t)(
            (uint16_t)data[5] |
            ((uint16_t)data[6] << 8)
        );

    uint8_t reference =
        data[7] & 0x03;


    // -------------------------------------------------
    // Heading
    // -------------------------------------------------

    if (headingRaw != 0xFFFF)
    {
        double headingRadians =
            headingRaw * 0.0001;

        double headingDegrees =
            headingRadians * 180.0 / PI;

        headingDegrees =
            fmod(
                headingDegrees,
                360.0
            );

        if (headingDegrees < 0)
        {
            headingDegrees += 360.0;
        }

        heading_.heading =
            headingDegrees;

        heading_.valid =
            true;

        heading_.source =
            HeadingSource::SCX20;  //TODO: need this

        heading_.lastUpdate =
            millis();
    }


    // -------------------------------------------------
    // Debug
    // -------------------------------------------------

    // Serial.println();

    // Serial.println(
    //     "========== 127250 =========="
    // );

    // Serial.print("SID: ");
    // Serial.println(sid);

    // Serial.print("Heading: ");
    // Serial.print(
    //     headingRaw * 0.0001 * 180.0 / PI,
    //     3
    // );

    // Serial.println(" deg");


    // if (deviationRaw != 0x7FFF)
    // {
    //     Serial.print("Deviation: ");

    //     Serial.print(
    //         deviationRaw * 0.0001 * 180.0 / PI,
    //         3
    //     );

    //     Serial.println(" deg");
    // }
    // else
    // {
    //     Serial.println(
    //         "Deviation: N/A"
    //     );
    // }


    // if (variationRaw != 0x7FFF)
    // {
    //     Serial.print("Variation: ");

    //     Serial.print(
    //         variationRaw * 0.0001 * 180.0 / PI,
    //         3
    //     );

    //     Serial.println(" deg");
    // }
    // else
    // {
    //     Serial.println(
    //         "Variation: N/A"
    //     );
    // }


    // Serial.print("Reference: ");

    // switch (reference)
    // {
    //     case 0:
    //         Serial.println("True");
    //         break;

    //     case 1:
    //         Serial.println("Magnetic");
    //         break;

    //     default:
    //         Serial.println("Reserved");
    //         break;
    // }

    // Serial.println(
    //     "============================"
    // );
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

        // Serial.print(
        //     "[SCX20] 129029 START seq="
        // );

        // Serial.print(
        //     sequence
        // );

        // Serial.print(
        //     " length="
        // );

        // Serial.println(
        //     messageLength
        // );


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

        // Serial.print(
        //     "[SCX20] 129029 frame error. Expected "
        // );

        // Serial.print(
        //     gnssPacket_.nextFrame
        // );

        // Serial.print(
        //     " got "
        // );

        // Serial.println(
        //     frame
        // );


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

        Serial.print(
            "[SCX20] 129029 DATA: "
        );

        for (uint8_t i = 0;
            i < gnssPacket_.receivedLength;
            i++)
        {
            if (gnssPacket_.data[i] < 0x10)
                Serial.print("0");

            Serial.print(
                gnssPacket_.data[i],
                HEX
            );
            Serial.print(" ");
        }


        // -------------------------------------------------
        // Decode complete 129029 payload
        // -------------------------------------------------

        Parse129029Payload(
            gnssPacket_.data,
            gnssPacket_.receivedLength
        );


        ResetFastPacket();
    }
}


// =====================================================
// Get Position
// =====================================================

bool SCX20CAN::GetPosition(  //TODO: Check if this function used?
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

void SCX20CAN::Parse129029Payload(const uint8_t* data, uint8_t length)
{
    // PGN 129029 should contain 43 bytes
    if (length < 43)
    {
        Serial.println(
            "[SCX20] 129029 payload too short"
        );

        return;
    }


    // =================================================
    // SID
    // =================================================

    uint8_t sid = data[0];


    // =================================================
    // UTC Date
    // =================================================
    //
    // Bytes 1-2
    // Resolution: 1 day
    //

    uint16_t daysSince1970 =
        (uint16_t)data[1] |
        ((uint16_t)data[2] << 8);


    // =================================================
    // UTC Time
    // =================================================
    //
    // Bytes 3-6
    // Resolution: 0.0001 second
    //

    uint32_t timeRaw =
        ((uint32_t)data[3]) |
        ((uint32_t)data[4] << 8) |
        ((uint32_t)data[5] << 16) |
        ((uint32_t)data[6] << 24);


    double utcTimeSeconds =
        timeRaw * 0.0001;

    uint32_t epoch =
        ((uint32_t)daysSince1970 * 86400UL)
        + (uint32_t)utcTimeSeconds;

    UTCDateTime utc = EpochToUTC(
                        daysSince1970,
                        utcTimeSeconds
                     );

    // =================================================
    // Latitude
    // =================================================
    //
    // Bytes 7-14
    // Resolution: 1e-16 degrees
    //

    int64_t latitudeRaw =
        ((int64_t)data[7]) |
        ((int64_t)data[8] << 8) |
        ((int64_t)data[9] << 16) |
        ((int64_t)data[10] << 24) |
        ((int64_t)data[11] << 32) |
        ((int64_t)data[12] << 40) |
        ((int64_t)data[13] << 48) |
        ((int64_t)data[14] << 56);


    double latitude =
        latitudeRaw * 1e-16;


    // =================================================
    // Longitude
    // =================================================
    //
    // Bytes 15-22
    // Resolution: 1e-16 degrees
    //

    int64_t longitudeRaw =
        ((int64_t)data[15]) |
        ((int64_t)data[16] << 8) |
        ((int64_t)data[17] << 16) |
        ((int64_t)data[18] << 24) |
        ((int64_t)data[19] << 32) |
        ((int64_t)data[20] << 40) |
        ((int64_t)data[21] << 48) |
        ((int64_t)data[22] << 56);


    double longitude =
        longitudeRaw * 1e-16;


    // =================================================
    // Altitude
    // =================================================
    //
    // Bytes 23-30
    // Resolution: 1e-6 metres
    //

    int64_t altitudeRaw =
        ((int64_t)data[23]) |
        ((int64_t)data[24] << 8) |
        ((int64_t)data[25] << 16) |
        ((int64_t)data[26] << 24) |
        ((int64_t)data[27] << 32) |
        ((int64_t)data[28] << 40) |
        ((int64_t)data[29] << 48) |
        ((int64_t)data[30] << 56);


    double altitude =
        altitudeRaw * 1e-6;


    // =================================================
    // GNSS information
    // =================================================

    uint8_t gnssType =
        data[31] & 0x0F;

    uint8_t method =
        (data[31] >> 4) & 0x0F;


    uint8_t integrity =
        data[32] & 0x03;


    // =================================================
    // Number of satellites
    // =================================================

    uint8_t satellites =
        data[33];


    // =================================================
    // HDOP
    // =================================================

    uint16_t hdopRaw =
        (uint16_t)data[34] |
        ((uint16_t)data[35] << 8);

    double hdop =
        hdopRaw * 0.01;


    // =================================================
    // PDOP
    // =================================================

    uint16_t pdopRaw =
        (uint16_t)data[36] |
        ((uint16_t)data[37] << 8);

    double pdop =
        pdopRaw * 0.01;


    // =================================================
    // Geoidal Separation
    // =================================================

    int32_t geoidalRaw =
        (int32_t)data[38] |
        ((int32_t)data[39] << 8) |
        ((int32_t)data[40] << 16) |
        ((int32_t)data[41] << 24);

    double geoidalSeparation =
        geoidalRaw * 0.01;


    // =================================================
    // Debug output
    // =================================================

    Serial.println();
    Serial.println(
        "========== 129029 =========="
    );

    Serial.print("SID: ");
    Serial.println(sid);

    Serial.print("UTC days: ");
    Serial.println(daysSince1970);

    Serial.print("UTC time: ");
    Serial.println(
        utcTimeSeconds,
        4
    );

    Serial.print("Latitude: ");
    Serial.println(
        latitude,
        10
    );

    Serial.print("Longitude: ");
    Serial.println(
        longitude,
        10
    );

    Serial.print("Altitude: ");
    Serial.println(
        altitude,
        3
    );

    Serial.print("GNSS Type: ");
    Serial.println(
        gnssType
    );

    Serial.print("GNSS Method: ");
    Serial.println(
        method
    );

    Serial.print("Integrity: ");
    Serial.println(
        integrity
    );

    Serial.print("Satellites: ");
    Serial.println(
        satellites
    );

    Serial.print("HDOP: ");
    Serial.println(
        hdop,
        2
    );

    Serial.print("PDOP: ");
    Serial.println(
        pdop,
        2
    );

    Serial.print("Geoidal separation: ");
    Serial.println(
        geoidalSeparation,
        2
    );

    Serial.println(
        "============================"
    );


    // =================================================
    // Populate PositionData
    // =================================================

    position_.latitude = latitude;
    position_.longitude = longitude;
    position_.altitude = altitude;

    //position_.utcDays = daysSince1970;
    //position_.utcTime = utcTimeSeconds;
    position_.timestamp = epoch;

    position_.year = utc.year;
    position_.month = utc.month;
    position_.day =  utc.day;
    position_.hour =  utc.hour;
    position_.minute =  utc.minute;
    position_.second =  utc.second;

    position_.satellites = satellites;
    position_.hdop = hdop;
    position_.pdop = pdop;

    position_.valid = true;

    position_.source = PositionSource::SCX20;

    position_.lastUpdate = millis();
}