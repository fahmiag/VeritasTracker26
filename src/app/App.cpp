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
 * 
 * 
*/


#include "App.h"
#include "config/Config.h"
#include <Arduino.h>

#include "config/PinMap.h"

//#include <avr/wdt.h> //TODO Change to ESP wdt
App::App()
    : sdLogger_(sdCard_)
{
}

void App::Begin()
{
    Serial.begin(115200);

    display_.Begin();
    buttons_.Begin();
    statusLED_.Begin();
    gpsManager_.Begin();
    sdCard_.Begin();
    //scx20_.Begin();
    if (sdCard_.IsReady())
    {
        sdLogger_.Begin();
    }

    statusLED_.SetMode(StatusLED::Mode::NORMAL);

    Serial.println("+++ START APP +++");

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
    gpsManager_.Update();
    //scx20_.Update();

    static uint32_t lastButtonScan = 0;

    uint32_t now = millis();

    if (now - lastButtonScan >= Config::BUTTON_SCAN_MS)
    {
        lastButtonScan = now;
        buttons_.Update();
        ButtonEvent event = buttons_.GetEvent();
        display_.Update(event);
    }

    static uint32_t lastLog = 0;

    if (millis() - lastLog >= 1000)
    {
        lastLog = millis();

        PositionData position;
        HeadingData heading;

        gpsManager_.GetPosition(position);
        headingManager_.GetHeading(heading);

        sdLogger_.LogNavigation(
            position,
            heading
        );
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
    
}

