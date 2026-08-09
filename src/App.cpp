#include "App.h"
#include "config/Config.h"
#include <Arduino.h>
//#include <avr/wdt.h> //TODO Change to ESP wdt

void App::Begin()
{
    Serial.begin(115200);

    display_.Begin();

    buttons_.Begin();

    display_.ShowMainScreen();

    Serial.println("+++ START APP +++");
}

void App::Run()
{
    uint32_t now = millis();

    // Button processing
    if (now - lastButtonUpdate_ >= Config::BUTTON_SCAN_MS)
    {
        lastButtonUpdate_ = now;

        buttons_.Update();

        //TODO: Comment/remove all display_.ShowMessage() in this function
        if (buttons_.WasPressed(Button::UP))
        {
            display_.ShowMessage("UP pressed");
            Serial.println("UP");
        }

        if (buttons_.WasPressed(Button::DOWN))
        {
            display_.ShowMessage("DOWN pressed");
            Serial.println("DOWN");
        }

        if (buttons_.WasPressed(Button::SELECT))
        {
            display_.ShowMessage("SELECT pressed");
            Serial.println("SELECT");
        }

        if (buttons_.WasPressed(Button::BACK))
        {
            display_.ShowMessage("BACK pressed");
            Serial.println("BACK");
        }
    }


    // Display processing
    // if (now - lastDisplayUpdate_ >= Config::DISPLAY_UPDATE_MS)
    // {
    //     lastDisplayUpdate_ = now;

    //     display_.Update();
    // }
    
}

