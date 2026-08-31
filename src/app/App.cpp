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
[x] RockBLOCK satellite communication
[ ] Main system state machine

[x] Test Create new file every day
[x] Pack Data
[x] Send data over-the-air
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
    : scx20_(),
      ublox_(),
      sdLogger_(sdCard_),
      lastLogTime_(0),
      satelliteManager_(rockBlock_),
      gpsManager_(scx20_, ublox_),
      telemetryStarted_(false),
      lastTelemetryTime_(0)
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
    
    if (rockBlock_.Begin())
    {
        Serial.println("RockBLOCK: OK");
    }
    else
    {
        Serial.println("RockBLOCK: ERROR");
    }

    //Done By GPS Manager
    // if (scx20_.Begin())
    // {
    //     Serial.println("SCX-20: OK");

    // }
    // else
    // {
    //     Serial.println("SCX-20: ERROR");
    // }


    satelliteManager_.Begin();

    //SendTestTelemetry();




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

    headingManager_.Update();
    


    // // -----------------------------------------
    // // Get current position
    // // -----------------------------------------

    // PositionData position;

    bool positionValid =
        gpsManager_.GetPosition(
            position_
        );


    // // -----------------------------------------
    // // Buttons
    // // -----------------------------------------

    
    buttons_.Update();

    ButtonEvent buttonEvent =
        buttons_.GetEvent();


    // // -----------------------------------------
    // // OLED
    // // -----------------------------------------

    display_.Update(
        buttonEvent,
        position_
    );

    if (!telemetryStarted_)
    {
        if (position_.valid)
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
    else if (now - lastTelemetryTime_ >= TELEMETRY_INTERVAL_MS)
    {
        Serial.println("\n5-minute telemetry interval reached");

        if (SendTelemetry())
        {
            Serial.println(
                "SendTelemetry() 5 minutes success");

            lastTelemetryTime_ = now;
        }
        else
        {
            Serial.println(
                "SendTelemetry() 5 minutes FAILED");
        }
    }
    else if (timeOutReset_ && (now - lastTelemetryTime_ >= TELEMETRY_TIMEOUT_MS))
    {
        rockBlock_.TimeOutReset();
        
        timeOutReset_ = true;
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


       

        //DEBUG===========================
        if (gpsManager_.GetHeading(heading_)) //do not delete
        {
            Serial.print("[APP] Heading: ");
            Serial.println(heading_.heading, 3);

            Serial.print("[APP] Heading valid: ");
            Serial.println(heading_.valid ? "YES" : "NO");

            Serial.print("[APP] Heading source: ");
            Serial.println(
                static_cast<uint8_t>(heading_.source)
            );
        }
        else
        {
            Serial.println("[APP] No valid heading");
        }


        //=================================


        // headingManager_.GetHeading(heading_);// not working
        //     //DEBUG===========================
        //     Serial.print("[APP] headingManager_.GetHeading: ");
        //     Serial.println(heading_.heading, 3);


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

     
    //put this at the bottom so that log and create file first
    rockBlock_.Update();
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
   

    packet.flags =
        Telemetry::NONE;

    if (position_.valid)
    {
        packet.flags |=
            Telemetry::GPS_VALID;
    }

    if (heading_.valid)
    {
        packet.flags |=
            Telemetry::HEADING_VALID;
    }

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

        //debug
         Serial.print("[APP]  HeadingData: ");
         Serial.println(heading_.heading, 3);

    packet.heading =
        static_cast<uint16_t>(
            heading_.heading * 100.0
        );

    if (heading_.valid)
    {
        packet.flags |= (1 << 2);
    }

        //debug
         Serial.print("[APP] TX Heading: ");
          Serial.println(packet.heading, 3);

    return satelliteManager_.SendTelemetry(packet);
}