#include "OLED.h"

//#include "../../config/PinMap.h"
#include <config/PinMap.h>

OLED::OLED()
    : display_(128, 64, &Wire, -1)
{
}

bool OLED::Begin()
{
    // Initialize I2C
    Wire.begin(PinMap::OLED_SDA, PinMap::OLED_SCL);

    // Initialize OLED
    if (!display_.begin(SSD1306_SWITCHCAPVCC, 0x3C)) //0x3C is the OLED I2C Address
    {
        return false;
    }

    display_.clearDisplay();

    display_.setTextSize(1);
    display_.setTextColor(SSD1306_WHITE);
    display_.setCursor(0, 0);

    display_.println("GPS TRACKER");
    display_.println();
    display_.println("Starting...");

    display_.display();

    return true;
}

void OLED::Clear()
{
    display_.clearDisplay();
}

void OLED::Update()
{
    display_.display();
}

void OLED::DisplayOn()
{
    display_.ssd1306_command(SSD1306_DISPLAYON);
}

void OLED::DisplayOff()
{
    display_.ssd1306_command(SSD1306_DISPLAYOFF);
}

void OLED::DrawText(uint8_t x, uint8_t y, const char* text)
{
    display_.setCursor(x, y);
    display_.print(text);
}

void OLED::DrawText(uint8_t x, uint8_t y, const String& text)
{
    display_.setCursor(x, y);
    display_.print(text);
}

void OLED::DrawLine(uint8_t x1, uint8_t y1,
                    uint8_t x2, uint8_t y2)
{
    display_.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
}

void OLED::SetTextColor(uint16_t color)
{
    display_.setTextColor(color);
}

void OLED::FillRect(
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    uint16_t color)
{
    display_.fillRect(x, y, width, height, color);
}