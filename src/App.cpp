/*TODO:
 * [ ] Display timeout
 * [ ] Add Led status blink
 * [ ] Read GPS from RockBlock
 * [ ] Pack Data
 * [ ] Send data over-the-air
 * [ ] Read Heading from Furuno
 * [ ] Read GPS from Furuno
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

    statusLED_.SetMode(StatusLED::Mode::NORMAL);

    Serial.println("+++ START APP +++");
}

void App::Run()
{
    statusLED_.Update();
    static uint32_t lastButtonScan = 0;

    uint32_t now = millis();

    if (now - lastButtonScan >= Config::BUTTON_SCAN_MS)
    {
        lastButtonScan = now;

        buttons_.Update();

        ButtonEvent event = buttons_.GetEvent();

        display_.Update(event);
    }

    
}

