#pragma once

#include "../drivers/OLED.h"
#include "../drivers/Buttons.h"
#include "../data/PositionData.h"

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

    //void Update(ButtonEvent event);
    void Update(
        ButtonEvent event,
        const PositionData& position
    );
    

    DisplayScreen GetScreen();



private:

    OLED oled_;

    // ---------------------------------
    // Current screen
    // ---------------------------------

    DisplayScreen currentScreen_ = DisplayScreen::MAIN;

    uint8_t menuIndex_ = 0;

    static constexpr uint8_t MENU_COUNT = 5;

    // ---------------------------------
    // GPS data
    // ---------------------------------

    PositionData position_;


    // ---------------------------------
    // OLED power management
    // ---------------------------------
    bool displayActive_ = true;

    uint32_t lastActivityTime_ = 0;

      // Current GPS position
   // PositionData position_;

    // ---------------------------------
    // Event handling
    // ---------------------------------

    void HandleEvent(ButtonEvent event);

    // ---------------------------------
    // Screen drawing
    // ---------------------------------

    void DrawScreen();
    void DrawMainScreen();
    void DrawMenu();
    void DrawGPSStatus();
    void DrawHeadingStatus();
    void DrawSatelliteStatus();
    void DrawSDStatus();
    void DrawSettings();

    // ---------------------------------
    // Menu navigation
    // ---------------------------------

    void NextMenuItem();
    void PreviousMenuItem();


    // ---------------------------------
    // OLED power management
    // ---------------------------------

    void WakeDisplay();
    void SleepDisplay();

    void ResetActivityTimer();
    void CheckDisplayTimeout();




};