#include "SCX20CAN.h"


SCX20CAN::SCX20CAN()
    : can_(CS_PIN)
{
}


bool SCX20CAN::Begin()
{
    position_ = PositionData{};

    position_.source =
        PositionSource::SCX20;


    heading_ = HeadingData{};

    heading_.source =
        HeadingSource::SCX20;


    // ESP32 SPI
    //
    // MOSI = GPIO23
    // MISO = GPIO19
    // SCK  = GPIO18
    // CS   = GPIO5

    SPI.begin(
        18,
        19,
        23,
        CS_PIN
    );


    Serial.println(
        "[SCX20] Initializing MCP2515..."
    );


    while (
        CAN_OK != can_.begin(
            CAN_500KBPS
        )
    )
    {
        Serial.println(
            "[SCX20] MCP2515 init failed"
        );

        Serial.println(
            "[SCX20] Retrying..."
        );

        delay(100);
    }


    Serial.println(
        "[SCX20] MCP2515 initialized"
    );


    // Normal CAN mode
    // can_.setMode(
    //     MCP_NORMAL
    // );


    Serial.println(
        "[SCX20] CAN started"
    );


    return true;
}

void SCX20CAN::Update()
{
    unsigned char len = 0;

    unsigned char buf[8];


    if (
        CAN_MSGAVAIL ==
        can_.checkReceive()
    )
    {
        can_.readMsgBuf(
            &len,
            buf
        );


        uint32_t canId =
            can_.getCanId();


        uint32_t pgn =
            ExtractPGN(
                canId
            );


        Serial.print(
            "[SCX20] CAN ID: 0x"
        );

        Serial.print(
            canId,
            HEX
        );


        Serial.print(
            " PGN: "
        );

        Serial.print(
            pgn
        );


        Serial.print(
            " DLC: "
        );

        Serial.print(
            len
        );


        Serial.print(
            " DATA: "
        );


        for (
            uint8_t i = 0;
            i < len;
            i++
        )
        {
            if (buf[i] < 0x10)
            {
                Serial.print("0");
            }

            Serial.print(
                buf[i],
                HEX
            );

            Serial.print(" ");
        }


        Serial.println();


        ProcessMessage(
            canId,
            buf,
            len
        );
    }
}

void SCX20CAN::ProcessMessage(
    uint32_t canId,
    uint8_t* data,
    uint8_t length
)
{
    uint32_t pgn =
        ExtractPGN(
            canId
        );


    switch (pgn)
    {
        case 127250:

            ParseVesselHeading(
                data,
                length
            );

            break;


        case 129025:

            ParsePositionRapidUpdate(
                data,
                length
            );

            break;


        case 129026:

            ParseCOGSOG(
                data,
                length
            );

            break;


        default:

            break;
    }
}

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
        pgn =
            ((uint32_t)dataPage << 16) |
            ((uint32_t)pduFormat << 8);
    }
    else
    {
        pgn =
            ((uint32_t)dataPage << 16) |
            ((uint32_t)pduFormat << 8) |
            pduSpecific;
    }


    return pgn;
}

void SCX20CAN::ParseVesselHeading(
    uint8_t* data,
    uint8_t length
)
{
    if (length < 8)
    {
        return;
    }


    uint16_t headingRaw =
        (uint16_t)data[1] |
        ((uint16_t)data[2] << 8);


    double headingRadians =
        headingRaw * 0.0001;


    double headingDegrees =
        headingRadians *
        180.0 /
        PI;


    if (headingDegrees >= 360.0)
    {
        headingDegrees -= 360.0;
    }


    heading_.heading =
        headingDegrees;


    heading_.valid =
        true;


    heading_.source =
        HeadingSource::SCX20;


    heading_.lastUpdate =
        millis();


    Serial.print(
        "[SCX20] Heading: "
    );

    Serial.print(
        headingDegrees
    );

    Serial.println(
        " deg"
    );
}

void SCX20CAN::ParsePositionRapidUpdate(
    uint8_t* data,
    uint8_t length
)
{
    if (length < 8)
    {
        return;
    }


    int32_t latRaw =
        (int32_t)data[0] |
        ((int32_t)data[1] << 8) |
        ((int32_t)data[2] << 16) |
        ((int32_t)data[3] << 24);


    int32_t lonRaw =
        (int32_t)data[4] |
        ((int32_t)data[5] << 8) |
        ((int32_t)data[6] << 16) |
        ((int32_t)data[7] << 24);


    position_.latitude =
        latRaw * 1e-7;


    position_.longitude =
        lonRaw * 1e-7;


    position_.valid =
        true;


    position_.source =
        PositionSource::SCX20;


    position_.lastUpdate =
        millis();


    Serial.print(
        "[SCX20] Latitude: "
    );

    Serial.println(
        position_.latitude,
        7
    );


    Serial.print(
        "[SCX20] Longitude: "
    );

    Serial.println(
        position_.longitude,
        7
    );
}

void SCX20CAN::ParseCOGSOG(
    uint8_t* data,
    uint8_t length
)
{
    if (length < 8)
    {
        return;
    }


    // COG

    uint16_t rawCOG =
        (uint16_t)data[2] |
        ((uint16_t)data[3] << 8);


    double cogRadians =
        rawCOG * 0.0001;


    double cogDegrees =
        cogRadians *
        180.0 /
        PI;


    if (cogDegrees >= 360.0)
    {
        cogDegrees -= 360.0;
    }


    position_.course =
        cogDegrees;


    // SOG

    uint16_t rawSOG =
        (uint16_t)data[4] |
        ((uint16_t)data[5] << 8);


    double speedMetersPerSecond =
        rawSOG * 0.01;


    position_.speed =
        speedMetersPerSecond *
        1.943844;


    position_.source =
        PositionSource::SCX20;


    position_.lastUpdate =
        millis();
}

bool SCX20CAN::GetPosition(
    PositionData& position
)
{
    position =
        position_;

    return position_.valid;
}


bool SCX20CAN::GetHeading(
    HeadingData& heading
)
{
    heading =
        heading_;

    return heading_.valid;
}