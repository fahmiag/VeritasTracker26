#pragma once

#include "../drivers/Display/OLED.h"
#include "../drivers/Button/Buttons.h"
#include "../drivers/Storage/SDCard.h"
#include "../drivers/Power/VoltageMonitor.h"

#include "../data/PositionData.h"
#include "../data/HeadingData.h"

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

struct SDStatus
{
    bool ready = false;
    bool logging = false;
};

struct SatelliteStatus
{
    bool ready = false;
    bool sending = false;
    bool lastTxSuccess = false;

    int8_t signal = -1;
};


class DisplayManager
{
public:

    bool Begin();

    //void Update(ButtonEvent event);
    void Update(
        ButtonEvent event,
        const PositionData& position,
        const HeadingData& heading,
        const SDStatus& sd,
        const SatelliteStatus& satellite,
        uint16_t reportingIntervalMinutes,
        const VoltageData& voltage
    );



    DisplayScreen GetScreen();

    void SetReportingIntervalMinutes(uint16_t minutes);



private:

    OLED oled_;

    // ---------------------------------
    // Current screen
    // ---------------------------------

    DisplayScreen currentScreen_ = DisplayScreen::MAIN;

    uint8_t menuIndex_ = 0;
    uint8_t screenIndex_ = 0;
    

    static constexpr uint8_t MENU_COUNT = 5;

    static constexpr uint8_t PAGE_COUNT = 5;

    // ---------------------------------
    // GPS data
    // ---------------------------------

    PositionData position_;
    HeadingData heading_;

    SDStatus sd_;
    SatelliteStatus satellite_;
    VoltageData voltage_;

    uint16_t reportingIntervalMinutes_ = 0;
    // ---------------------------------
    // OLED power management
    // ---------------------------------
    bool displayActive_ = true;

    uint32_t lastActivityTime_ = 0;
    uint32_t lastDisplayUpdate_ = 0;

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
    
    void UpdateDisplay();

    // ---------------------------------
    // Menu navigation
    // ---------------------------------

    void NextMenuItem();
    void PreviousMenuItem();

    // ---------------------------------
    // Page navigation
    // ---------------------------------

    void NextPageItem();
    void PreviousPageItem();

    void GotoPage();

    // ---------------------------------
    // OLED power management
    // ---------------------------------

    void WakeDisplay();
    void SleepDisplay();


    void CheckDisplayTimeout();



};