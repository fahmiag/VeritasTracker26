#include "Buttons.h"

#include "../config/PinMap.h"

bool Buttons::Begin()
{
    pinMode(PinMap::BUTTON_UP, INPUT_PULLUP);
    pinMode(PinMap::BUTTON_DOWN, INPUT_PULLUP);
    pinMode(PinMap::BUTTON_SELECT, INPUT_PULLUP);
    pinMode(PinMap::BUTTON_BACK, INPUT_PULLUP);

    return true;
}

//This gives one event when a button is pressed, 
//rather than continuously reporting the button while it is held.
Button Buttons::Read()
{
    if (digitalRead(PinMap::BUTTON_UP) == LOW && lastStateUp_ == HIGH)
    {
        lastStateUp_ = LOW;
        Serial.println("Button UP");
        return Button::UP;
    }
    if (digitalRead(PinMap::BUTTON_UP) == HIGH)
    {
        lastStateUp_ = HIGH;
    }


    if (digitalRead(PinMap::BUTTON_DOWN) == LOW && lastStateDown_ == HIGH)
    {
        lastStateDown_ = LOW;
        Serial.println("Button DOWN");
        return Button::DOWN;
    }
    if (digitalRead(PinMap::BUTTON_DOWN) == HIGH)
    {
        lastStateDown_ = HIGH;
    }


    if (digitalRead(PinMap::BUTTON_SELECT) == LOW && lastStateSelect_ == HIGH)
    {
        lastStateSelect_ = LOW;
        Serial.println("Button SELECT");
        return Button::SELECT;
    }
    if (digitalRead(PinMap::BUTTON_SELECT) == HIGH)
    {
        lastStateSelect_ = HIGH;
    }


    if (digitalRead(PinMap::BUTTON_BACK) == LOW && lastStateBack_ == HIGH)
    {
        lastStateBack_ = LOW;
        Serial.println("Button BACK");
        return Button::BACK;
    }
    if (digitalRead(PinMap::BUTTON_BACK) == HIGH)
    {
        lastStateBack_ = HIGH;
    }

    return Button::NONE;
}