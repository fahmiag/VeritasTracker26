#pragma once
#include <Arduino.h>
//hardware pin assignments

namespace PinMap
{
    // ------------------------------------------------
    // OLED
    // ------------------------------------------------
    constexpr uint8_t OLED_SDA = 21;
    constexpr uint8_t OLED_SCL = 22;

    // ------------------------------------------------
    // Buttons
    // ------------------------------------------------
    constexpr uint8_t BUTTON_UP     = 14;
    constexpr uint8_t BUTTON_DOWN   = 13;
    constexpr uint8_t BUTTON_SELECT = 25;
    constexpr uint8_t BUTTON_BACK   = 26;

    // ------------------------------------------------
    // Status LED & Buzzer
    // ------------------------------------------------
    constexpr uint8_t BUZZER_PIN = 12; 
    constexpr uint8_t LED_STATUS = 15;

    // ------------------------------------------------
    // u-blox M10 GNSS
    // ------------------------------------------------
    
    constexpr uint8_t UBLOX_RX = 16; // GPIO16 = RX2;
    constexpr uint8_t UBLOX_TX = 17; // GPIO17 = TX2;

    
   // const int SD_CS_PIN = 4;
   // const int MCP_CS_PIN = 5;

}