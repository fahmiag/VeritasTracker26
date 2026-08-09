#include <Arduino.h>

//application settings,
//one central configuration file.
//No magic numbers elsewhere.

namespace Config
{
    constexpr uint32_t BUTTON_SCAN_MS = 20;
    constexpr uint32_t BUTTON_DEBOUNCE_MS = 50;
    constexpr uint32_t BUTTON_LONG_PRESS_MS = 1000;

    //constexpr uint32_t SATELLITE_INTERVAL_MS = 60000;

    //constexpr uint32_t GPS_BAUD = 115200;
    //constexpr uint32_t GPS_UPDATE_MS = 100;

    //constexpr uint32_t HEADING_BAUD = 4800;

    //constexpr uint32_t SAT_BAUD = 19200;

    //constexpr uint32_t LOG_INTERVAL_MS = 1000;

    //constexpr uint32_t TX_INTERVAL_MS = 60000;
}