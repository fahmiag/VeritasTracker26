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

class Buttons
{
public:

    bool Begin();

    Button Update();
    //Button Read();

private:
    struct ButtonState 
    { 
        uint8_t pin; 
        Button button; 
        bool stableState; 
        bool lastReading; 
        uint32_t lastChangeTime; 
    }; 
    
    ButtonState buttons_[4];

    // Button ReadButton(uint8_t pin, Button button);

    // uint8_t lastStateUp_ = HIGH;
    // uint8_t lastStateDown_ = HIGH;
    // uint8_t lastStateSelect_ = HIGH;
    // uint8_t lastStateBack_ = HIGH;
};