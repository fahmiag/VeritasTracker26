#include "SCX20CAN.h"

#include "../config/PinMap.h"


bool SCX20CAN::Begin()
{
    position_ = PositionData{};
    position_.source = PositionSource::SCX20;

    heading_ = HeadingData{};
    heading_.source = HeadingSource::SCX20;


    twai_general_config_t generalConfig =
        TWAI_GENERAL_CONFIG_DEFAULT(
            (gpio_num_t)PinMap::SCX20_CAN_TX,
            (gpio_num_t)PinMap::SCX20_CAN_RX,
            TWAI_MODE_NORMAL
        );


    twai_timing_config_t timingConfig =
        TWAI_TIMING_CONFIG_250KBITS(); //TWAI_TIMING_CONFIG_250KBITS(); //500KBITS

    twai_filter_config_t filterConfig =
        TWAI_FILTER_CONFIG_ACCEPT_ALL();


    esp_err_t result = twai_driver_install(
        &generalConfig,
        &timingConfig,
        &filterConfig
    );

    if (result != ESP_OK)
    {
        Serial.print("CAN driver install failed: "); 
        Serial.println(result);

        return false;
    }


    result = twai_start();

    if (result != ESP_OK)
    {
        Serial.print("CAN start failed: "); 
        Serial.println(result);
        return false;
    }

    Serial.println("[SCX20] CAN started");
    return true;
}


void SCX20CAN::Update()
{
    twai_message_t message;


    while (twai_receive(&message, 0) == ESP_OK)
    {
        Serial.print("CAN ID: 0x"); 
        Serial.print( message.identifier, HEX ); 
        Serial.print(" ");

        if (message.extd)
        {
            ProcessMessage(message);
            Serial.print("EXT");
        }
        else { Serial.print("STD"); }

        Serial.print(" DLC: "); 
        Serial.print( message.data_length_code ); 
        Serial.print(" DATA: "); 
        
        if (!message.rtr) 
        { 
            for ( int i = 0; i < message.data_length_code; i++ ) 
            { 
                if (message.data[i] < 0x10) 
                { 
                    Serial.print("0"); 
                } 
                
                Serial.print( message.data[i], HEX ); 
                Serial.print(" "); 
            } 
        } 
        Serial.println();
    }
}


uint32_t SCX20CAN::ExtractPGN(uint32_t canId)
{
    uint8_t pf = (canId >> 16) & 0xFF;
    uint8_t ps = (canId >> 8) & 0xFF;


    // PDU1
    // Destination-specific PGN
    if (pf < 240)
    {
        return ((uint32_t)pf << 8);
    }


    // PDU2
    // Global PGN
    return ((uint32_t)pf << 8) | ps;
}


void SCX20CAN::ProcessMessage(
    const twai_message_t& message)
{
    uint32_t pgn =
        ExtractPGN(message.identifier);


    switch (pgn)
    {
        case 127250:
            ParseVesselHeading(message);
            break;


        case 129025:
            ParsePositionRapidUpdate(message);
            break;


        case 129026:
            ParseCOGSOG(message);
            break;


        default:
            break;
    }
}

void SCX20CAN::ParseVesselHeading(
    const twai_message_t& message)
{
    if (message.data_length_code < 5)
    {
        return;
    }


    // Bytes 1-2 = Heading
    uint16_t rawHeading =
        message.data[1] |
        ((uint16_t)message.data[2] << 8);


    // Resolution = 0.0001 radians
    double headingRadians =
        rawHeading * 0.0001;


    double headingDegrees =
        headingRadians * 180.0 / PI;


    // Normalize to 0-360
    if (headingDegrees >= 360.0)
    {
        headingDegrees -= 360.0;
    }


    heading_.heading =
        headingDegrees;


    heading_.valid = true;

    heading_.source =
        HeadingSource::SCX20;

    heading_.lastUpdate =
        millis();
}

void SCX20CAN::ParsePositionRapidUpdate(
    const twai_message_t& message)
{
    if (message.data_length_code < 8)
    {
        return;
    }


    int32_t rawLatitude =
        (int32_t)(
            message.data[0] |
            ((uint32_t)message.data[1] << 8) |
            ((uint32_t)message.data[2] << 16) |
            ((uint32_t)message.data[3] << 24)
        );


    int32_t rawLongitude =
        (int32_t)(
            message.data[4] |
            ((uint32_t)message.data[5] << 8) |
            ((uint32_t)message.data[6] << 16) |
            ((uint32_t)message.data[7] << 24)
        );


    position_.latitude =
        rawLatitude * 1e-7;


    position_.longitude =
        rawLongitude * 1e-7;


    position_.valid = true;

    position_.source =
        PositionSource::SCX20;
}

void SCX20CAN::ParseCOGSOG(
    const twai_message_t& message)
{
    if (message.data_length_code < 7)
    {
        return;
    }


    // COG
    uint16_t rawCOG =
        message.data[2] |
        ((uint16_t)message.data[3] << 8);


    // Resolution = 0.0001 radians
    double cogRadians =
        rawCOG * 0.0001;


    double cogDegrees =
        cogRadians * 180.0 / PI;


    if (cogDegrees >= 360.0)
    {
        cogDegrees -= 360.0;
    }


    position_.course =
        cogDegrees;


    // SOG
    uint16_t rawSOG =
        message.data[4] |
        ((uint16_t)message.data[5] << 8);


    // Resolution = 0.01 m/s
    double speedMetersPerSecond =
        rawSOG * 0.01;


    // Convert m/s → knots
    position_.speed =
        speedMetersPerSecond * 1.943844;


    position_.source =
        PositionSource::SCX20;
}

bool SCX20CAN::GetPosition(
    PositionData& position)
{
    position = position_;

    return position_.valid;
}


bool SCX20CAN::GetHeading(
    HeadingData& heading)
{
    heading = heading_;

    return heading_.valid;
}