#include "OLED.h"

#include "../config/PinMap.h"

OLED::OLED()
    : display_(U8G2_R0,
               U8X8_PIN_NONE,
               PinMap::OLED_SCL,
               PinMap::OLED_SDA)
{
}

bool OLED::Begin()
{
    if (!display_.begin())
    {
        return false;
    }

    display_.setFont(u8g2_font_6x10_tf);

    display_.clearBuffer();

    display_.drawStr(0, 12, "GPS TRACKER");
    display_.drawStr(0, 28, "Starting...");

    display_.sendBuffer();

    return true;
}

void OLED::Clear()
{
    display_.clearBuffer();
}

void OLED::Update()
{
    display_.sendBuffer();
}

void OLED::DrawText(uint8_t x, uint8_t y, const char* text)
{
    display_.drawStr(x, y, text);
}

void OLED::DrawText(uint8_t x, uint8_t y, const String& text)
{
    display_.drawStr(x, y, text.c_str());
}

void OLED::DrawLine(uint8_t x1, uint8_t y1,
                    uint8_t x2, uint8_t y2)
{
    display_.drawLine(x1, y1, x2, y2);
}