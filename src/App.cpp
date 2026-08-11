/*TODO:
 * [x] Display timeout
 * [x] Add Led status blink
 * [ ] Read GPS from RockBlock
 * [ ] Pack Data
 * [ ] Send data over-the-air
 * [ ] Read Heading from Furuno
 * [ ] Read GPS from Furuno
 * [ ] Safe dat into sd card csv
 * [ ] Add Buzzer to indicate Error
 * 
 * 
*/


#include "App.h"
#include "config/Config.h"
#include <Arduino.h>

#include "config/PinMap.h"

//#include <avr/wdt.h> //TODO Change to ESP wdt

void App::Begin()
{
    Serial.begin(115200);

    display_.Begin();
    buttons_.Begin();
    statusLED_.Begin();
    gpsManager_.Begin();

    statusLED_.SetMode(StatusLED::Mode::NORMAL);

    Serial.println("+++ START APP +++");
}

void App::Run()
{
    statusLED_.Update();
    gpsManager_.Update();

    static uint32_t lastButtonScan = 0;
    static uint32_t lastGPSPrint = 0;

    uint32_t now = millis();

    if (now - lastButtonScan >= Config::BUTTON_SCAN_MS)
    {
        lastButtonScan = now;

        buttons_.Update();

        ButtonEvent event = buttons_.GetEvent();

        display_.Update(event);
    }

    

    if (millis() - lastGPSPrint >= 1000)
    {
        lastGPSPrint = millis();

        PositionData position;

        if (gpsManager_.GetPosition(position))
        {
            Serial.print("GPS: ");

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

    
}

