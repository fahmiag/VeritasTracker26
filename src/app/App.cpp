/*
TODO:

[x] Display timeout
[x] Add Led status blink
[x] Read GPS from RockBlock
[x] Save data into 1 day logfile
[x] Micro SD CSV logger
[x] OLED navigation/status pages

[ ] SCX20 CAN
[ ] GPSManager source selection
[ ] Navigation/Heading data structure
[ ] RockBLOCK satellite communication
[ ] Main system state machine

[x] Test Create new file every day
[ ] Pack Data
[ ] Send data over-the-air
[ ] Read Heading from Furuno
[ ] Read GPS from Furuno
[ ] Add Buzzer to indicate Error
[ ] Add Millis to Serial Monitor
[ ] Add debug.h
*/


#include "App.h"

#include "../config/Config.h"
#include "../config/PinMap.h"

#include <Arduino.h>


// =====================================================
// Constructor
// =====================================================

App::App()
    : sdLogger_(sdCard_),
      lastLogTime_(0),
      satelliteManager_(rockBlock_)
{
}


// =====================================================
// Begin
// =====================================================

void App::Begin()
{
    Serial.begin(115200);

    delay(500);

    if (rockBlock_.Begin())
    {
        Serial.println("RockBLOCK: OK");
    }
    else
    {
        Serial.println("RockBLOCK: ERROR");
    }

    // rockBlock_.SendText(
    //     RAW_TOPIC,
    //     "Hello from application!"
    // );

    satelliteManager_.Begin();

    //SendTestTelemetry();

    Serial.println();
    Serial.println("==============================");
    Serial.println(" GPS Tracker Starting");
    Serial.println("==============================");


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
    // SCX-20 CAN
    // -----------------------------------------

    /*
     * SCX-20 is currently being
     * investigated separately.
     *
     * Leave the driver disabled for now.
     */

    Serial.println("SCX-20: disabled");


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
    // Heading Manager
    // -----------------------------------------

    if (!headingManager_.Begin())
    {
        Serial.println(
            "Heading Manager: ERROR"
        );
    }
    else
    {
        Serial.println(
            "Heading Manager: OK"
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

    statusLED_.SetMode(
        StatusLED::Mode::NORMAL
    );


    Serial.println("==============================");
    Serial.println("System ready");
    Serial.println("==============================");


    // -----------------------------------------
    // SD card test
    // -----------------------------------------

    /*
     * Temporary SD card test.
     *
     * Can remove this after SD card
     * hardware has been verified.
     */

    if (sdCard_.IsReady())
    {
        Serial.println();
        Serial.println("SD TEST");


        sdCard_.WriteFile(
            "/test.txt",
            "Hello from ESP32\n"
        );


        sdCard_.AppendFile(
            "/test.txt",
            "SD card is working\n"
        );


        sdCard_.ReadFile(
            "/test.txt"
        );
    }
}


// =====================================================
// Run
// =====================================================

void App::Run()
{
    // -----------------------------------------
    // Current time
    // -----------------------------------------

    uint32_t now = millis();

    rockBlock_.Update();
    satelliteManager_.Update();


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

    // headingManager_.Update();


    // // -----------------------------------------
    // // Get current position
    // // -----------------------------------------

    // PositionData position;

    bool positionValid =
        gpsManager_.GetPosition(
            position
        );


    // // -----------------------------------------
    // // Buttons
    // // -----------------------------------------

    
    // buttons_.Update();

    // ButtonEvent buttonEvent =
    //     buttons_.GetEvent();


    // // -----------------------------------------
    // // OLED
    // // -----------------------------------------

    // display_.Update(
    //     buttonEvent,
    //     position
    // );

    if (now - lastTelemetryTime_ >= TELEMETRY_INTERVAL_MS)
    {
        lastTelemetryTime_ = now;

        SendTelemetry();
        Serial.println("SendTelemetry()");
    }


    // // -----------------------------------------
    // // Print GPS information
    // // -----------------------------------------

    // // static uint32_t lastGPSPrint = 0;


    // // if (
    // //     now - lastGPSPrint >= 1000
    // // )
    // // {
    // //     lastGPSPrint = now;


    // //     if (positionValid)
    // //     {
    // //         Serial.print(
    // //             "uBlox POS: "
    // //         );


    // //         Serial.print(
    // //             position.latitude,
    // //             6
    // //         );


    // //         Serial.print(
    // //             ", "
    // //         );


    // //         Serial.print(
    // //             position.longitude,
    // //             6
    // //         );


    // //         Serial.print(
    // //             "  SAT="
    // //         );


    // //         Serial.print(
    // //             position.satellites
    // //         );


    // //         Serial.print(
    // //             "  HDOP="
    // //         );


    // //         Serial.println(
    // //             position.hdop
    // //         );
    // //     }
    // //     else
    // //     {
    // //         Serial.println(
    // //             "GPS: NO FIX"
    // //         );
    // //     }
    // // }


    // // -----------------------------------------
    // // Logging
    // // -----------------------------------------

    if (
        now - lastLogTime_
        >= LOG_INTERVAL_MS
    )
    {
        lastLogTime_ = now;


        HeadingData heading;


        headingManager_.GetHeading(
            heading
        );


        if (positionValid)
        {
            sdLogger_.LogNavigation(
                position,
                heading
            );
        }
    }


    // // -----------------------------------------
    // // SD Logger periodic flush
    // // -----------------------------------------

     sdLogger_.Update();
}

void App::SendTelemetry()
{
    if (!position.valid)
    {
        Serial.println(
            "[SAT] GPS position invalid"
        );

        return;
    }

    TelemetryPacket packet{};

    packet.version =
        Telemetry::VERSION;

    packet.type =
        static_cast<uint8_t>(
            Telemetry::Type::POSITION
        );

    packet.flags =
        Telemetry::NONE;

    if (position.valid)
    {
        packet.flags |=
            Telemetry::GPS_VALID;
    }

    if (heading.valid)
    {
        packet.flags |=
            Telemetry::HEADING_VALID;
    }

    packet.timestamp =
        DateTimeToUnix(
            position.year,
            position.month,
            position.day,
            position.hour,
            position.minute,
            position.second
        );

    packet.latitude =
        static_cast<int32_t>(
            position.latitude * 10000000.0
        );

    packet.longitude =
        static_cast<int32_t>(
            position.longitude * 10000000.0
        );

    packet.cog =
        static_cast<uint16_t>(
            position.course * 100.0
        );

    packet.heading =
        static_cast<uint16_t>(
            heading.heading * 100.0
        );

    satelliteManager_.SendTelemetry(packet);
}