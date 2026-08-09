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

    Button Read();

private:

    Button ReadButton(uint8_t pin, Button button);

    uint8_t lastStateUp_ = HIGH;
    uint8_t lastStateDown_ = HIGH;
    uint8_t lastStateSelect_ = HIGH;
    uint8_t lastStateBack_ = HIGH;
};