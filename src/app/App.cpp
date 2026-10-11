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

#include <cmath>


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

    statusLED_.Begin();

    voltageMonitor_.Begin();

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
    // Alarm manager
    // -----------------------------------------
    alarmManager_.Begin();


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
     voltageMonitor_.Update();


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

    // GetPosition() can fail without replacing position_.
    // Also reject a fix older than five seconds.
    positionValid =
        positionValid &&
        uint32_t(now - position_.lastUpdate) <= 5000UL;

    position_.valid = positionValid;

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

    alarmManager_.Update(
        BuildTelemetryPacket()
    );


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

    // Returns false while the input is active or its alert is outstanding.
    // bool emergencyCleared =
    //     alarmManager_.AcknowledgeEmergency();

    // bool tamperCleared =
    //     alarmManager_.AcknowledgeTamper();

    // // -----------------------------------------
    // // Voltage
    // // -----------------------------------------
    voltageMonitor_.Update();
    
    // // -----------------------------------------
    // // OLED
    // // -----------------------------------------

    display_.Update(
        buttonEvent,
        position_,
        heading_,
        sdStatus_,
        satelliteStatus_,
        configuration_.GetReportingIntervalMinutes(),
        voltageMonitor_.GetData()
    );


    // -------------------------------------------------
// Consume completion of a previously queued alarm.
// -------------------------------------------------

bool alarmSuccess = false;

if (satelliteManager_.ConsumeAlarmResult(alarmSuccess))
{
    alarmManager_.Complete(alarmSuccess);

    Serial.println(
        alarmSuccess
            ? "[ALARM] Transmission succeeded"
            : "[ALARM] Transmission failed; retry pending"
    );
}

// -------------------------------------------------
// Alarm messages have priority over routine positions.
// -------------------------------------------------

if (satelliteManager_.IsReady() &&
    !satelliteManager_.IsBusy())
{
    TelemetryPacket alarmPacket{};

    bool firstAttempt = false;

    if (alarmManager_.GetNext(
            alarmPacket,
            firstAttempt))
    {
        if (satelliteManager_.SendAlarm(
                alarmPacket,
                firstAttempt))
        {
            alarmManager_.Queued(alarmPacket);

            Serial.println(
                alarmPacket.type == Telemetry::Type::EMERGENCY
                    ? "[ALARM] Emergency message queued"
                    : "[ALARM] Tamper message queued"
            );
        }
        else
        {
            alarmManager_.QueueFailed(
                alarmPacket.type
            );
        }
    }
}

// -------------------------------------------------
// Periodic positions retain their own reporting timer.
// -------------------------------------------------

const uint32_t telemetryIntervalMs =
    configuration_.GetReportingIntervalMs();

if (telemetryIntervalMs == 0)
{
    telemetryStarted_ = false;
}
else if (!satelliteManager_.IsBusy())
{
    const bool firstDue =
        !telemetryStarted_ &&
        now >= FIRST_TELEMETRY_DELAY_MS;

    const bool periodicDue =
        telemetryStarted_ &&
        uint32_t(now - lastTelemetryTime_) >=
            telemetryIntervalMs;

    if ((firstDue || periodicDue) &&
        positionValid &&
        SendTelemetry())
    {
        telemetryStarted_ = true;

        lastTelemetryTime_ = now;
    }
}

    // const uint32_t telemetryIntervalMs =
    //     configuration_.GetReportingIntervalMs();

    // if (telemetryIntervalMs == 0)
    // {
    //     telemetryStarted_ = false;
    // }
    // else if (!telemetryStarted_) //TODO; set to send the first telemetry after 1 minutes, 
    //                              //need to wait for heading
    // {
    //     const bool startupDelayElapsed =
    //     now >= FIRST_TELEMETRY_DELAY_MS;


    //         // Wait at least one minute and require valid navigation data.
    //     if (startupDelayElapsed &&
    //         positionValid)
    //     {
    //         if (SendTelemetry())
    //         {
    //             Serial.println(
    //                 "First telemetry queued successfully"
    //             );

    //             telemetryStarted_ = true;
    //             lastTelemetryTime_ = now;
    //             timeOutReset_ = false;
    //         }
    //         else
    //         {
    //             Serial.println(
    //                 "First telemetry FAILED"
    //             );
    //         }
    //     }
    // }
    // else if (now - lastTelemetryTime_ >= telemetryIntervalMs)
    // {
    //     Serial.println("\nTelemetry interval reached");

    //     const uint32_t telemetryIntervalMinutes = 
    //     configuration_.GetReportingIntervalMinutes();

    //     if (SendTelemetry())
    //     {
    //         Serial.print("[APP] SendTelemetry() ");
    //         Serial.print(telemetryIntervalMinutes);
    //         Serial.println(" minutes interval success");

    //         lastTelemetryTime_ = now;
    //     }
    //     else
    //     {
    //         Serial.print("[APP] SendTelemetry() ");
    //         Serial.print(telemetryIntervalMinutes);
    //         Serial.println(" minutes interval FAILED");
    //     }
    // }
    // else if (timeOutReset_ && (now - lastTelemetryTime_ >= TELEMETRY_TIMEOUT_MS))
    // {
    //     rockBlock_.TimeOutReset();
        
    //     timeOutReset_ = true;
    // }


 
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
TelemetryPacket App::BuildTelemetryPacket() const
{
    TelemetryPacket packet{};

    packet.version = Telemetry::VERSION;
    packet.type = Telemetry::Type::POSITION;

    packet.positionSource =
        position_.source == PositionSource::SCX20
            ? Telemetry::PositionSource::SCX20
            : Telemetry::PositionSource::UBLOX;

    const bool validPosition =
        position_.valid &&
        std::isfinite(position_.latitude) &&
        std::isfinite(position_.longitude) &&
        position_.latitude >= -90.0 &&
        position_.latitude <= 90.0 &&
        position_.longitude >= -180.0 &&
        position_.longitude <= 180.0;

    if (validPosition)
    {
        packet.status |= Telemetry::POSITION_VALID;

        packet.latitude =
            static_cast<int32_t>(
                position_.latitude * 10000000.0
            );

        packet.longitude =
            static_cast<int32_t>(
                position_.longitude * 10000000.0
            );

        if (std::isfinite(position_.course) &&
            position_.course >= 0.0 &&
            position_.course < 360.0)
        {
            packet.status |= Telemetry::COG_VALID;

            packet.cog =
                static_cast<uint16_t>(
                    position_.course * 100.0
                );
        }
    }

    if (heading_.valid &&
        std::isfinite(heading_.heading) &&
        heading_.heading >= 0.0 &&
        heading_.heading < 360.0)
    {
        packet.status |= Telemetry::HEADING_VALID;

        packet.heading =
            static_cast<uint16_t>(
                heading_.heading * 100.0
            );
    }

    const bool dateInRange =
        position_.valid &&
        position_.year >= 2000 &&
        position_.year <= 2099 &&
        position_.month >= 1 &&
        position_.month <= 12 &&
        position_.day >= 1 &&
        position_.hour < 24 &&
        position_.minute < 60 &&
        position_.second < 60;

    if (dateInRange)
    {
        const uint8_t daysInMonth[] =
        {
            31, 28, 31, 30, 31, 30,
            31, 31, 30, 31, 30, 31
        };

        uint8_t maximumDay =
            daysInMonth[position_.month - 1];

        // Correct for the supported range 2000–2099.
        if (position_.month == 2 &&
            position_.year % 4 == 0)
        {
            maximumDay = 29;
        }

        if (position_.day <= maximumDay)
        {
            packet.status |= Telemetry::TIME_VALID;

            packet.timestamp =
                DateTimeToUnix(
                    position_.year,
                    position_.month,
                    position_.day,
                    position_.hour,
                    position_.minute,
                    position_.second
                );
        }
    }

    packet.status =
        alarmManager_.ApplyStatus(packet.status);

    return packet;
}

bool App::SendTelemetry()
{
    const TelemetryPacket packet =
        BuildTelemetryPacket();

    if (!(packet.status & Telemetry::POSITION_VALID))
    {
        return false;
    }

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