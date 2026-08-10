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

    void DisplayOn();
    void DisplayOff();

    void DrawText(uint8_t x, uint8_t y, const char* text);

    void DrawText(uint8_t x, uint8_t y, const String& text);

    void DrawLine(uint8_t x1, uint8_t y1,
                  uint8_t x2, uint8_t y2);

    void SetTextColor(uint16_t color);
    
    void FillRect(uint8_t x, uint8_t y,
              uint8_t width, uint8_t height,
              uint16_t color);

private:

    Adafruit_SSD1306 display_;
};