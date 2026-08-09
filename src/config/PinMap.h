#pragma once
#include <Arduino.h>
//hardware pin assignments

namespace PinMap
{
    // OLED
    constexpr uint8_t OLED_SDA = 21;
    constexpr uint8_t OLED_SCL = 22;

    // Buttons
    constexpr uint8_t BUTTON_UP     = 14;
    constexpr uint8_t BUTTON_DOWN   = 13;
    constexpr uint8_t BUTTON_SELECT = 25;
    constexpr uint8_t BUTTON_BACK   = 26;
}