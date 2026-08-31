#pragma once

// Managers
#include "../managers/DisplayManager.h"
#include "../managers/ButtonManager.h"
#include "../managers/GPSManager.h"
#include "../managers/HeadingManager.h"

// Drivers
#include "../drivers/GPS/UBloxGNSS.h"
#include "../drivers/CAN/SCX20CAN.h"
#include "../drivers/StatusLED.h"
#include "../drivers/Storage/SDCard.h"
#include "../drivers/Satellite/RockBlockPlus9704.h"

#include "../managers/SatelliteManager.h"
#include "../protocol/TelemetryConfig.h"
#include "../data/TelemetryPacket.h"

#include "../utils/TimeUtils.h"
// Services
#include "../services/SDLogger.h"
#include <Arduino.h>


#include "../config/Debug.h"

//App coordinates modules but doesn't implement hardware details.
//App never talks directly to hardware.


class App
{
public:
    App();

    void Begin();
    void Run();
    void Update(); //test code

private:

    PositionData position_;
    HeadingData heading_;
    // -----------------------------
    // Hardware drivers
    // -----------------------------

    RockBlockPlus9704 rockBlock_;
    SatelliteManager satelliteManager_;

    UBloxGNSS ublox_;
    SCX20CAN scx20_;
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

    uint32_t lastTelemetryTime_;

    #if DEBUG_SATELLITE_TX
        static constexpr uint32_t TELEMETRY_INTERVAL_MS =5UL * 60UL * 1000UL;
    #else
        static constexpr uint32_t TELEMETRY_INTERVAL_MS =1UL * 60UL * 1000UL;
    #endif



    static constexpr uint32_t TELEMETRY_TIMEOUT_MS = 10UL * 60UL * 1000UL;

    bool SendTelemetry();
    bool telemetryStarted_;
    bool timeOutReset_;

    // GPSManager gps;
    // HeadingManager heading;
    // SatelliteManager sat;
    // LoggerManager logger;
    // DisplayManager display;
    // ButtonManager buttons;

};