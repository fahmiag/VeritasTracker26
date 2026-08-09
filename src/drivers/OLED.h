#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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

    Adafruit_SSD1306 display_;
};