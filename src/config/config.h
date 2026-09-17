#include <Arduino.h>

//application settings,
//one central configuration file.
//No magic numbers elsewhere.

namespace Config
{
    //Keypad
    constexpr uint32_t BUTTON_SCAN_MS = 20;
    constexpr uint32_t BUTTON_DEBOUNCE_MS = 50;
    constexpr uint32_t BUTTON_LONG_PRESS_MS = 1000;

    constexpr uint32_t OLED_TIMEOUT_MS = 60000; //60000ms = 1minutes
    constexpr uint32_t OLED_UPDATE_MS = 1000; 

    //LED Status
    static constexpr uint32_t NORMAL_ON_MS  = 1000;
    static constexpr uint32_t NORMAL_OFF_MS = 2000;
    static constexpr uint32_t ERROR_INTERVAL_MS  = 200;

    constexpr uint8_t TELEMETRY_VERSION = 1;
    constexpr uint8_t TELEMETRY_TYPE_POSITION = 1;


}