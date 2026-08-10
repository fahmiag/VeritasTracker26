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
    

    DisplayScreen GetScreen();



private:

    OLED oled_;

    DisplayScreen currentScreen_ = DisplayScreen::MAIN;

    uint8_t menuIndex_ = 0;

    static constexpr uint8_t MENU_COUNT = 5;

    bool displayActive_ = true;

    uint32_t lastActivityTime_ = 0;

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



    void WakeDisplay();
    void SleepDisplay();

    void ResetActivityTimer();
    void CheckDisplayTimeout();




};