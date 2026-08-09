#include "App.h"
#include "config/Config.h"
#include <Arduino.h>
//#include <avr/wdt.h> //TODO Change to ESP wdt

void App::Begin()
{
    Serial.begin(115200);

    display_.Begin();

    buttons_.Begin();

   // display_.ShowMainScreen();

    Serial.println("+++ START APP +++");
}

void App::Run()
{
    static uint32_t lastButtonScan = 0;

    uint32_t now = millis();

    if (now - lastButtonScan >= Config::BUTTON_SCAN_MS)
    {
        lastButtonScan = now;

        buttons_.Update();

        ButtonEvent event = buttons_.GetEvent();

        display_.Update(event);
    }

    // Button processing
    // if (now - lastButtonUpdate_ >= Config::BUTTON_SCAN_MS)
    // {
    //     lastButtonUpdate_ = now;

    //     buttons_.Update();

    //     ButtonEvent event = buttons_.GetEvent();


    //     switch (event)
    //     {
    //         case ButtonEvent::UP_SHORT:
    //             Serial.println("UP SHORT");
    //             break;

    //         case ButtonEvent::UP_LONG:
    //             Serial.println("UP LONG");
    //             break;

    //         case ButtonEvent::DOWN_SHORT:
    //             Serial.println("DOWN SHORT");
    //             break;

    //         case ButtonEvent::DOWN_LONG:
    //             Serial.println("DOWN LONG");
    //             break;

    //         case ButtonEvent::SELECT_SHORT:
    //             Serial.println("SELECT SHORT");
    //             break;

    //         case ButtonEvent::SELECT_LONG:
    //             Serial.println("SELECT LONG");
    //             break;

    //         case ButtonEvent::BACK_SHORT:
    //             Serial.println("BACK SHORT");
    //             break;

    //         case ButtonEvent::BACK_LONG:
    //             Serial.println("BACK LONG");
    //             break;

    //         case ButtonEvent::NONE:
    //             break;
    //     }
    // }


    // Display processing
    // if (now - lastDisplayUpdate_ >= Config::DISPLAY_UPDATE_MS)
    // {
    //     lastDisplayUpdate_ = now;

    //     display_.Update();
    // }
    
}

