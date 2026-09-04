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
    
    // RockBLOCK is initialized by SatelliteManager.

    //Done By GPS Manager
    // if (scx20_.Begin())
    // {
    //     Serial.println("SCX-20: OK");

    // }
    // else
    // {
    //     Serial.println("SCX-20: ERROR");
    // }
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

    const uint32_t telemetryIntervalMs =
        configuration_.GetReportingIntervalMs();

    if (telemetryIntervalMs == 0)
    {
        telemetryStarted_ = false;
    }
    else if (!telemetryStarted_)
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
    else if (now - lastTelemetryTime_ >= telemetryIntervalMs)
    {
        Serial.println("\nTelemetry interval reached");

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
    // // Logging
    // // -----------------------------------------

    if (
        now - lastLogTime_
        >= LOG_INTERVAL_MS
    )
    {
        lastLogTime_ = now;


        if (gpsManager_.GetHeading(heading_)) //do not delete
        {
            #if DEBUG_SCX20_HDG
                Serial.print("[APP] Heading: ");
                Serial.println(heading_.heading, 3);
            #endif

        }
        else
        {
            Serial.println("[APP] No valid heading");
        }




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

    if (position_.valid)
    {
        packet.status |= Telemetry::POSITION_VALID;
    }

    if (position_.valid)
    {
        packet.status |= Telemetry::COG_VALID;
    }

    if (heading_.valid)
    {
        packet.status |= Telemetry::HEADING_VALID;
    }

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

        //debug
         Serial.print("[APP]  HeadingData: ");
         Serial.println(heading_.heading, 3);

    packet.heading =
        static_cast<uint16_t>(
            heading_.heading * 100.0
        );


        //debug
         Serial.print("[APP] TX Heading: ");
          Serial.println(packet.heading, 3);

    return satelliteManager_.SendTelemetry(packet);
}