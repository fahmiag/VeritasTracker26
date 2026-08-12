/*TODO:
 * [x] Display timeout
 * [x] Add Led status blink
 * [x] Read GPS from RockBlock
 * 
 * //Suggested by chat GPT
 * [ ] SCX20 CAN
 * [ ] GPSManager source selection
 * [ ] Navigation/Heading data structure
 * [ ] Micro SD CSV logger
 * [ ] OLED navigation/status pages
 * [ ] RockBLOCK satellite communication
 * [ ] Main system state machine
 * 
 * [ ] Pack Data
 * [ ] Send data over-the-air
 * [ ] Read Heading from Furuno
 * [ ] Read GPS from Furuno
 * [ ] Safe dat into sd card csv
 * [ ] Add Buzzer to indicate Error
 * [ ] Add Millis to Serial Monitor
 * [ ] Add debug.h
 * 
*/


#include "App.h"
#include "config/Config.h"
#include <Arduino.h>

#include "config/PinMap.h"

//#include <avr/wdt.h> //TODO Change to ESP wdt
App::App()
    :   sdLogger_(sdCard_), 
        lastLogTime_(0)

{
}

void App::Begin()
{
    Serial.begin(115200);

    delay(500); 
    Serial.println(); 
    Serial.println("=============================="); 
    Serial.println(" GPS Tracker Starting"); 
    Serial.println("==============================");

    display_.Begin();
    buttons_.Begin();
    statusLED_.Begin();

    // -------------------------------- 
    // u-blox GNSS 
    // --------------------------------

    // if (!ublox_.Begin()) 
    // { 
    //     Serial.println("u-blox: ERROR"); 
    // } 
    // else 
    // { 
    //     Serial.println("u-blox: OK"); 
    // }

    // -------------------------------- 
    // SCX-20 CAN 
    // -------------------------------- 
    /* * SCX-20 is currently being 
    * investigated separately. 
    * 
    * * Leave the driver disabled for now. 
    * */ 

    Serial.println("SCX-20: disabled");

    // -------------------------------- 
    // GPS Manager 
    // -------------------------------- 
    if (!gpsManager_.Begin()) 
    { 
        Serial.println("GPS Manager: ERROR"); 
    } 
    else 
    { 
        Serial.println("GPS Manager: OK"); 
    }
    
    // -------------------------------- 
    // Heading Manager 
    // -------------------------------- 
    if (!headingManager_.Begin()) 
    { 
        Serial.println("Heading Manager: ERROR"); 
    } 
    else 
    { 
        Serial.println("Heading Manager: OK"); 
    }
    
    // -------------------------------- 
    // SD Card 
    // --------------------------------
    if (sdCard_.Begin())
    {
        Serial.println("SD Card: OK");
        if (sdLogger_.Begin())
        {
            Serial.println("SD Logger: OK");
        }
        else
        {
            Serial.println("SD Logger: ERROR");
        }
    }
    else
    {
        Serial.println("SD Card: ERROR");
    }

    statusLED_.SetMode(StatusLED::Mode::NORMAL);

    Serial.println("==============================");
    Serial.println("System ready");
    Serial.println("==============================");

    if (sdCard_.IsReady())
    {
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

void App::Run()
{
    statusLED_.Update();
    //scx20CAN_.Update();
   // ublox_.Update();

    gpsManager_.Update();
    headingManager_.Update();

    sdLogger_.Update();

    static uint32_t lastButtonScan = 0;

    uint32_t now = millis();

    if (now - lastButtonScan >= Config::BUTTON_SCAN_MS)
    {
        lastButtonScan = now;
        buttons_.Update();
        ButtonEvent event = buttons_.GetEvent();
        display_.Update(event);
    }


    static uint32_t lastGPSPrint = 0;
    if (millis() - lastGPSPrint >= 1000)
    {
        lastGPSPrint = millis();

        PositionData position;

        if (gpsManager_.GetPosition(position))
        {
            Serial.print("uBlox POS: ");
            Serial.print(position.latitude, 6);
            Serial.print(", ");
            Serial.print(position.longitude, 6);
            Serial.print("  SAT=");
            Serial.print(position.satellites);
            Serial.print("  HDOP=");
            Serial.println(position.hdop);
        }
        else
        {
            Serial.println("GPS: NO FIX");
        }
    }

        // Log once per second
    if (
        now - lastLogTime_
        >= LOG_INTERVAL_MS
    )
    {
        lastLogTime_ = now;

        PositionData position;
        HeadingData heading;
        bool positionValid = gpsManager_.GetPosition(position); 
        headingManager_.GetHeading( heading);

        if (positionValid)
        {
            sdLogger_.LogNavigation(position, heading);
        }
    }

    // PositionData scxPosition;

    // if (scx20_.GetPosition(scxPosition))
    // {
    //     Serial.print("SCX20 POS: ");

    //     Serial.print(scxPosition.latitude, 7);
    //     Serial.print(", ");

    //     Serial.print(scxPosition.longitude, 7);

    //     Serial.print(" SOG=");
    //     Serial.print(scxPosition.speed);

    //     Serial.print(" COG=");
    //     Serial.println(scxPosition.course);
    // }

    // HeadingData heading;

    // if (scx20_.GetHeading(heading))
    // {
    //     Serial.print("SCX20 HEADING: ");

    //     Serial.println(
    //         heading.heading,
    //         2
    //     );
    // }

    // Handles periodic flush
    sdLogger_.Update();
    
}

