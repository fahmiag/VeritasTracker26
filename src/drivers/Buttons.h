#pragma once

#include <Arduino.h>

enum class Button
{
    NONE,
    UP,
    DOWN,
    SELECT,
    BACK
};

enum class ButtonEvent 
{ 
    NONE, 

    UP_SHORT, 
    UP_LONG, 

    DOWN_SHORT, 
    DOWN_LONG, 

    SELECT_SHORT, 
    SELECT_LONG, 

    BACK_SHORT, 
    BACK_LONG 
};

class Buttons
{
public:

    bool Begin();

    ButtonEvent Update();
    //Button Read();

private:
    struct ButtonState 
    { 
        uint8_t pin; 
        Button button; 
        bool stableState; 
        bool lastReading; 
        uint32_t lastChangeTime; 
        uint32_t pressStartTime; 
        bool longPressReported;
    }; 
    
    ButtonState buttons_[4];

    ButtonEvent MakeEvent(
        Button button, 
        bool longPress 
    );


};