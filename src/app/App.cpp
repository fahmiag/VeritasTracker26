/*
TODO:

[x] Display timeout
[x] Add Led status blink
[x] Read GPS from RockBlock
[x] Save data into 1 day logfile
[x] Micro SD CSV logger
[x] OLED navigation/status pages
[x] SCX20 CAN
[x] Navigation/Heading data structure
[x] RockBLOCK satellite communication
[x] Test Create new file every day
[x] Pack Data
[x] Send data over-the-air
[x] Read Heading from Furuno
[x] Read GPS from Furuno
[x] Add debug.h

[ ] Main system state machine
[ ] GPSManager source selection
[ ] Add Buzzer to indicate Error
[ ] Send first position report after 1 minutes
[ ] Check why the first position report does not have heading

*/


#include "App.h"

#include "../config/Config.h"
#include "../config/PinMap.h"

#include <Arduino.h>
#include <SPI.h>


// =====================================================
// Constructor
// =====================================================

App::App()
    : configuration_(),
      rockBlock_(),
      scx20_(),
      ublox_(),
      sdLogger_(sdCard_),
      lastLogTime_(0),
      satelliteManager_(rockBlock_, configuration_),
      gpsManager_(scx20_, ublox_),
      telemetryStarted_(false),
      lastTelemetryTime_(0),
      timeOutReset_(false)
{
} 


// =====================================================
// Begin
// =====================================================


void App::Begin()
{
    Serial.begin(115200);

    delay(500);

    Serial.println();
    Serial.println("==============================");
    Serial.println(" GPS Tracker Starting");
    Serial.println("==============================");



    // =================================================
    // Shared SPI Bus
    //
    // Devices:
    //   SD Card  -> CS GPIO4
    //   MCP2515  -> CS GPIO5
    // =================================================

     InitializeSPI();

    // -----------------------------------------
    // Configuration Manager
    // -----------------------------------------

    if (configuration_.Begin())
    {
        Serial.print("Configuration: OK, telemetry interval = ");
        Serial.print(configuration_.GetReportingIntervalMinutes());
        Serial.println(" minute(s)");
    }
    else
    {
        Serial.println("Configuration: ERROR, using defaults");
    }


    if (satelliteManager_.Begin())
    {
        Serial.println("Satellite Manager : OK");
    }
    else
    {
        Serial.println("Satellite Manager: ERROR");
    }


    // -----------------------------------------
    // Display
    // -----------------------------------------

    if (display_.Begin())
    {
        Serial.println("Display: OK");
    }
    else
    {
        Serial.println("Display: ERROR");
    }


    // -----------------------------------------
    // Buttons
    // -----------------------------------------

    if (buttons_.Begin())
    {
        Serial.println("Buttons: OK");
    }
    else
    {
        Serial.println("Buttons: ERROR");
    }


    // -----------------------------------------
    // Status LED
    // -----------------------------------------

    if (statusLED_.Begin())
    {
        Serial.println("Status LED: OK");
    }
    else
    {
        Serial.println("Status LED: ERROR");
    }





    // -----------------------------------------
    // GPS Manager
    // -----------------------------------------

    if (!gpsManager_.Begin())
    {
        Serial.println(
            "GPS Manager: ERROR"
        );
    }
    else
    {
        Serial.println(
            "GPS Manager: OK"
        );
    }



    
    // -----------------------------------------
    // SD Card
    // -----------------------------------------

    if (sdCard_.Begin())
    {
        Serial.println(
            "SD Card: OK"
        );


        if (sdLogger_.Begin())
        {
            Serial.println(
                "SD Logger: OK"
            );
        }
        else
        {
            Serial.println(
                "SD Logger: ERROR"
            );
        }
    }
    else
    {
        Serial.println(
            "SD Card: ERROR"
        );
    }


    // -----------------------------------------
    // Normal LED operation
    // -----------------------------------------

    // statusLED_.SetMode(
    //     StatusLED::Mode::NORMAL
    // );

    Serial.println("==============================");
    Serial.println("System ready");
    Serial.println("==============================");


}


// ##############################################################
// Run
// ##############################################################

void App::Run()
{
    // -----------------------------------------
    // Current time
    // -----------------------------------------

    uint32_t now = millis();


    // // -----------------------------------------
    // // Status LED
    // // -----------------------------------------

     statusLED_.Update();


    // // -----------------------------------------
    // // GPS
    // // -----------------------------------------

     gpsManager_.Update();


    // // -----------------------------------------
    // // Heading
    // // -----------------------------------------

  //  headingManager_.Update();
    


    // // -----------------------------------------
    // // Get current position
    // // -----------------------------------------

    // PositionData position;

    bool positionValid =
        gpsManager_.GetPosition(
            position_
        );

    if (!gpsManager_.GetHeading(heading_))
    {
        heading_.valid = false;

        #if DEBUG_SCX20_HDG
            Serial.println("[APP] No valid heading");
        #endif
    }
    else
    {
        heading_.valid = true;

        #if DEBUG_SCX20_HDG
            Serial.print("[APP] Heading: ");
            Serial.println(heading_.heading, 3);
        #endif

    }


    // // -----------------------------------------
    // // Buttons
    // // -----------------------------------------

    
    buttons_.Update();

    ButtonEvent buttonEvent =
        buttons_.GetEvent();

    // -----------------------------------------
    // Display status data
    // -----------------------------------------

    sdStatus_.ready = sdCard_.IsReady();

    sdStatus_.logging = sdLogger_.IsReady();

    satelliteStatus_.ready =
        satelliteManager_.IsReady();

    satelliteStatus_.sending =
        satelliteManager_.IsSending();

    satelliteStatus_.lastTxSuccess =
        satelliteManager_.WasLastSendSuccessful();

    satelliteStatus_.signal = -1; //current RockBLOCK wrapper does not expose signal strength yet.


    // // -----------------------------------------
    // // OLED
    // // -----------------------------------------

    display_.Update(
        buttonEvent,
        position_,
        heading_,
        sdStatus_,
        satelliteStatus_,
        configuration_.GetReportingIntervalMinutes()
    );

    const uint32_t telemetryIntervalMs =
        configuration_.GetReportingIntervalMs();

    

    if (telemetryIntervalMs == 0)
    {
        telemetryStarted_ = false;
    }
    else if (!telemetryStarted_) //TODO; set to send the first telemetry after 1 minutes, 
                                 //need to wait for heading
    {
        const bool startupDelayElapsed =
        now >= FIRST_TELEMETRY_DELAY_MS;


            // Wait at least one minute and require valid navigation data.
        if (startupDelayElapsed &&
            positionValid)
        {
            if (SendTelemetry())
            {
                Serial.println(
                    "First telemetry queued successfully"
                );

                telemetryStarted_ = true;
                lastTelemetryTime_ = now;
                timeOutReset_ = false;
            }
            else
            {
                Serial.println(
                    "First telemetry FAILED"
                );
            }
        }
    }
    else if (now - lastTelemetryTime_ >= telemetryIntervalMs)
    {
        Serial.println("\nTelemetry interval reached");

        const uint32_t telemetryIntervalMinutes = 
        configuration_.GetReportingIntervalMinutes();

        if (SendTelemetry())
        {
            Serial.print("[APP] SendTelemetry()");
            Serial.print(telemetryIntervalMinutes);
            Serial.println(" minutes interval success");

            lastTelemetryTime_ = now;
        }
        else
        {
            Serial.print("[APP] SendTelemetry()");
            Serial.print(telemetryIntervalMinutes);
            Serial.println(" minutes interval FAILED");
        }
    }
    else if (timeOutReset_ && (now - lastTelemetryTime_ >= TELEMETRY_TIMEOUT_MS))
    {
        rockBlock_.TimeOutReset();
        
        timeOutReset_ = true;
    }


 
    // // -----------------------------------------
    // // Logging
    // // -----------------------------------------

    if (now - lastLogTime_ >= LOG_INTERVAL_MS)
    {
        lastLogTime_ = now;


        //TODO: Add heading not valid
        // if (gpsManager_.GetHeading(heading_)) //do not delete
        // {
        //     #if DEBUG_SCX20_HDG
        //         Serial.print("[APP] Heading: ");
        //         Serial.println(heading_.heading, 3);
        //     #endif

        // }
        // else
        // {
        //     #if DEBUG_SCX20_HDG
        //         Serial.println("[APP] No valid heading");
        //     #endif
        // }


        if (positionValid)
        {
            sdLogger_.LogNavigation(
                position_,
                heading_
            );
        }
    }


    // // -----------------------------------------
    // // SD Logger periodic flush
    // // -----------------------------------------

     sdLogger_.Update();

     
    // Put this at the bottom so that logging/file creation runs first.
    // SatelliteManager owns RockBLOCK polling.
    satelliteManager_.Update();
}

bool App::SendTelemetry()
{
    if (!position_.valid)
    {
        Serial.println(
            "[SAT] GPS position invalid"
        );

        return false;
    }

    TelemetryPacket packet{};

    packet.version =
        Telemetry::VERSION;
 
    packet.type =
            Telemetry::Type::POSITION;
   

    packet.status = 0;

    //TODO: try remove if (position_.valid)

    // if (position_.valid)
    // {
    //     packet.status |= Telemetry::POSITION_VALID;
    // }

    // if (position_.valid)
    // {
    //     packet.status |= Telemetry::COG_VALID;
    // }

    // if (heading_.valid)
    // {
    //     packet.status |= Telemetry::HEADING_VALID;
    // }
    packet.status |= Telemetry::POSITION_VALID;
    packet.status |= Telemetry::COG_VALID;
    packet.status |= Telemetry::HEADING_VALID;

    if (position_.year >= 1970 &&
        position_.month >= 1 && position_.month <= 12 &&
        position_.day >= 1 && position_.day <= 31)
    {
        packet.status |= Telemetry::TIME_VALID;
    }

    packet.positionSource =
        (position_.source == PositionSource::SCX20)
            ? Telemetry::PositionSource::SCX20
            : Telemetry::PositionSource::UBLOX;

    packet.timestamp =
        DateTimeToUnix(
            position_.year,
            position_.month,
            position_.day,
            position_.hour,
            position_.minute,
            position_.second
        );

    packet.latitude =
        static_cast<int32_t>(
            position_.latitude * 10000000.0
        );

    packet.longitude =
        static_cast<int32_t>(
            position_.longitude * 10000000.0
        );

    packet.cog =
        static_cast<uint16_t>(
            position_.course * 100.0
        );



    // packet.heading =
    //     static_cast<uint16_t>(
    //         heading_.heading * 100.0
    //     );

    packet.heading = heading_.valid
        ? static_cast<uint16_t>(heading_.heading * 100.0)
        : 0;


    return satelliteManager_.SendTelemetry(packet);
}

void App::InitializeSPI()
{
    // -------------------------------------------------
    // Disable all SPI devices
    // -------------------------------------------------

    pinMode(
        PinMap::SD_CS,
        OUTPUT
    );

    pinMode(
        PinMap::SCX20_CAN_CS,
        OUTPUT
    );


    digitalWrite(
        PinMap::SD_CS,
        HIGH
    );

    digitalWrite(
        PinMap::SCX20_CAN_CS,
        HIGH
    );


    // -------------------------------------------------
    // Shared VSPI bus
    // -------------------------------------------------

    SPI.begin(
        PinMap::SPI_SCK,
        PinMap::SPI_MISO,
        PinMap::SPI_MOSI
    );


    Serial.println(
        "Shared SPI Bus: initialized"
    );
}