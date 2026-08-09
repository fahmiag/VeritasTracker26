#include "App.h"
#include "config/Config.h"
#include <Arduino.h>
//#include <avr/wdt.h> //TODO Change to ESP wdt

void App::Begin()
{
 
    display_.Begin();
    buttons_.Begin();

    display_.ShowMainScreen();
}

void App::Run()
{
    uint32_t now = millis();

    // Button processing
    if (now - lastButtonUpdate_ >= Config::BUTTON_SCAN_MS)
    {
        lastButtonUpdate_ = now;

        buttons_.Update();

        if (buttons_.WasPressed(Button::UP))
        {
            display_.ShowMessage("UP pressed");
        }

        if (buttons_.WasPressed(Button::DOWN))
        {
            display_.ShowMessage("DOWN pressed");
        }

        if (buttons_.WasPressed(Button::SELECT))
        {
            display_.ShowMessage("SELECT pressed");
        }

        if (buttons_.WasPressed(Button::BACK))
        {
            display_.ShowMessage("BACK pressed");
        }
    }


    // Display processing
    if (now - lastDisplayUpdate_ >= Config::DISPLAY_UPDATE_MS)
    {
        lastDisplayUpdate_ = now;

        display_.Update();
    }
    
}

