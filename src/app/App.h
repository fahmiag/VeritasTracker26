#pragma once

// Managers
#include "../managers/DisplayManager.h"
#include "../managers/ButtonManager.h"
#include "../managers/GPSManager.h"
#include "../managers/HeadingManager.h"

// Drivers
#include "../drivers/UBloxGNSS.h"
#include "../drivers/SCX20CAN.h"
#include "../drivers/StatusLED.h"
#include "../drivers/SDCard.h"
#include "../drivers/RockBlockPlus9704.h"

// Services
#include "../services/SDLogger.h"

//App coordinates modules but doesn't implement hardware details.
//App never talks directly to hardware.


class App
{
public:
    App();

    void Begin();
    void Run();

private:


    // -----------------------------
    // Hardware drivers
    // -----------------------------

    RockBlockPlus9704 rockBlock_;

   // UBloxGNSS ublox_;
    SCX20CAN scx20CAN_;
    SDCard sdCard_;

    // -----------------------------
    // Managers
    // -----------------------------
    DisplayManager display_;
    ButtonManager buttons_;
    GPSManager gpsManager_;
    HeadingManager headingManager_;

    StatusLED statusLED_;


    // -----------------------------
    // Services
    // -----------------------------
    SDLogger sdLogger_;

    // -----------------------------
    // Logging
    // -----------------------------
    
    uint32_t lastLogTime_;

    static constexpr uint32_t
        LOG_INTERVAL_MS = 1000;



    uint32_t lastDisplayUpdate_ = 0;
    uint32_t lastButtonUpdate_ = 0;

    // GPSManager gps;
    // HeadingManager heading;
    // SatelliteManager sat;
    // LoggerManager logger;
    // DisplayManager display;
    // ButtonManager buttons;

};