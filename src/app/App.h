#pragma once

#include "../managers/DisplayManager.h"
#include "../managers/ButtonManager.h"
#include "../drivers/StatusLED.h"
#include "../managers/GPSManager.h"
#include "../managers/HeadingManager.h"
#include "../drivers/SDCard.h"
#include "../drivers/SDCard.h"
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

    DisplayManager display_;
    ButtonManager buttons_;
    StatusLED statusLED_;

    UBloxGNSS ublox_;
    SCX20CAN scx20_;

    SDCard sdCard_;
    SDLogger sdLogger_;

    GPSManager gpsManager_;
    HeadingManager headingManager_;

    uint32_t lastDisplayUpdate_ = 0;
    uint32_t lastButtonUpdate_ = 0;

    // GPSManager gps;
    // HeadingManager heading;
    // SatelliteManager sat;
    // LoggerManager logger;
    // DisplayManager display;
    // ButtonManager buttons;

};