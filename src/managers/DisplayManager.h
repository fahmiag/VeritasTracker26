#pragma once

#include "../drivers/OLED.h"
#include "../drivers/Buttons.h"

enum class DisplayScreen
{
    MAIN,
    MENU,
    GPS_STATUS,
    HEADING_STATUS,
    SATELLITE_STATUS,
    SD_STATUS,
    SETTINGS
};

class DisplayManager
{
public:

    bool Begin();

    void Update(ButtonEvent event);
    //void Update();

    DisplayScreen GetScreen();

    //void ShowMainScreen();
    //void ShowMessage(const char* message);

private:

    OLED oled_;

        DisplayScreen currentScreen_ = DisplayScreen::MAIN;

    uint8_t menuIndex_ = 0;

    static constexpr uint8_t MENU_COUNT = 5;

    void HandleEvent(ButtonEvent event);

    void DrawScreen();

    void DrawMainScreen();

    void DrawMenu();

    void DrawGPSStatus();

    void DrawHeadingStatus();

    void DrawSatelliteStatus();

    void DrawSDStatus();

    void DrawSettings();

    void NextMenuItem();

    void PreviousMenuItem();
};