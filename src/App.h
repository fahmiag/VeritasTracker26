#pragma once

#include "managers/DisplayManager.h"
#include "managers/ButtonManager.h"
#include "drivers/StatusLED.h"
#include "managers/GPSManager.h"

//App coordinates modules but doesn't implement hardware details.
//App never talks directly to hardware.

class App
{
public:

    void Begin();
    void Run();

private:

    DisplayManager display_;
    ButtonManager buttons_;
    StatusLED statusLED_;
    GPSManager gpsManager_;

    uint32_t lastDisplayUpdate_ = 0;
    uint32_t lastButtonUpdate_ = 0;

    // GPSManager gps;
    // HeadingManager heading;
    // SatelliteManager sat;
    // LoggerManager logger;
    // DisplayManager display;
    // ButtonManager buttons;

};