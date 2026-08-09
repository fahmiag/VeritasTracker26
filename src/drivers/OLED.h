#pragma once

#include <Arduino.h>
#include <U8g2lib.h>  //suggest by chatGPT
//#include <Adafruit_SSD1306.h> //previously use this

class OLED
{
public:

    OLED();

    bool Begin();

    void Clear();

    void Update();

    void DrawText(uint8_t x, uint8_t y, const char* text);

    void DrawText(uint8_t x, uint8_t y, const String& text);

    void DrawLine(uint8_t x1, uint8_t y1,
                  uint8_t x2, uint8_t y2);

private:

    U8G2_SSD1306_128X64_NONAME_F_HW_I2C display_;
};