#include <Arduino.h>

namespace Config
{
    constexpr uint32_t GPS_BAUD = 115200;

    constexpr uint32_t HEADING_BAUD = 4800;

    constexpr uint32_t SAT_BAUD = 19200;

    constexpr uint32_t LOG_INTERVAL_MS = 1000;

    constexpr uint32_t TX_INTERVAL_MS = 60000;
}