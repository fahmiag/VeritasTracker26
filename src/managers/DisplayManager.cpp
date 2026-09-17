#include "DisplayManager.h"
#include "../config/Config.h"


bool DisplayManager::Begin()
{
    if (!oled_.Begin())
    {
        return false;
    }


    position_ = PositionData{};
    heading_ = HeadingData{};
    sd_ = SDStatus{};
    satellite_ = SatelliteStatus{};


    currentScreen_ =
        DisplayScreen::MAIN;

    screenIndex_ = 0;

    displayActive_ = true;

    lastActivityTime_ = millis();




    DrawScreen();


    return true;
}


// =====================================================
// Update
// =====================================================

void DisplayManager::Update(
    ButtonEvent event,
    const PositionData& position,
    const HeadingData& heading,
    const SDStatus& sd,
    const SatelliteStatus& satellite,
    uint16_t reportingIntervalMinutes,
    const VoltageData& voltage
)
{
    // Store latest data
    position_  = position;
    heading_   = heading;
    sd_        = sd;
    satellite_ = satellite;
    voltage_ = voltage;

    reportingIntervalMinutes_ = reportingIntervalMinutes;

    // Check OLED timeout
    CheckDisplayTimeout();
    UpdateDisplay();

    // No button event
    if (event == ButtonEvent::NONE) 
    {
        return;
    }
    
    // -----------------------------------------
    // OLED currently OFF
    // -----------------------------------------

    if (!displayActive_)
    {
        if (event == ButtonEvent::BACK_SHORT ||
            event == ButtonEvent::BACK_LONG)
        {
            currentScreen_ = DisplayScreen::MAIN;
        }

        WakeDisplay();
       // Event is consumed.
        //
        // SELECT wakes current page.
        // BACK wakes MAIN page.
        return;
    }


    // -----------------------------------------
    // Display already active
    // -----------------------------------------

    //ResetActivityTimer(); //Need this?
    lastActivityTime_ = millis();
    
    HandleEvent(event);
}




void DisplayManager::UpdateDisplay()
{

    uint32_t now =
        millis();


    if (
        (now - lastDisplayUpdate_) >=
        Config::OLED_UPDATE_MS
    )
    {
        lastDisplayUpdate_=now;
        DrawScreen();
    }
}



// =====================================================
// Display Timeout
// =====================================================

void DisplayManager::CheckDisplayTimeout()
{
    if (!displayActive_)
    {
        return;
    }


    uint32_t now =
        millis();


    if (
        (now - lastActivityTime_) >=
        Config::OLED_TIMEOUT_MS
    )
    {
        SleepDisplay();
    }
}


// =====================================================
// Sleep Display
// =====================================================

void DisplayManager::SleepDisplay()
{
    oled_.DisplayOff();

    displayActive_ = false;
}


// =====================================================
// Wake Display
// =====================================================

void DisplayManager::WakeDisplay()
{
    oled_.DisplayOn();

    displayActive_ = true;

    lastActivityTime_ =
        millis();


    DrawScreen();
}


// =====================================================
// Handle Button Event
// =====================================================

void DisplayManager::HandleEvent(
    ButtonEvent event)
{
    switch (currentScreen_)
    {
        // -----------------------------------------
        // MAIN SCREEN
        // -----------------------------------------

        case DisplayScreen::MAIN:

            if (event == ButtonEvent::SELECT_SHORT) 
            {

                GotoPage();       
                DrawScreen();
            }

            break;


     

        // -----------------------------------------
        // SUBMENU SCREENS
        // -----------------------------------------

        case DisplayScreen::GPS_STATUS:

        case DisplayScreen::HEADING_STATUS:

        case DisplayScreen::SATELLITE_STATUS:

        case DisplayScreen::SD_STATUS:

        case DisplayScreen::SETTINGS:

        case DisplayScreen::VOLTAGE:

        case DisplayScreen::ERROR:

            if (
                event == ButtonEvent::BACK_SHORT ||
                event == ButtonEvent::BACK_LONG)
            {
                currentScreen_ =
                    DisplayScreen::MAIN;

                DrawScreen();
            }

            if (event == ButtonEvent::UP_SHORT)
            {
                PreviousPageItem();
                GotoPage();
            }

            else if (event == ButtonEvent::DOWN_SHORT)
            {
                
                NextPageItem();
                GotoPage();
            }

            break;
    }
}

void DisplayManager::GotoPage()
{
    switch (screenIndex_)
    {
        case 0:
            currentScreen_ = DisplayScreen::GPS_STATUS;

            break;

        case 1:
            currentScreen_ = DisplayScreen::HEADING_STATUS;

            break;

        case 2:
            currentScreen_ = DisplayScreen::SATELLITE_STATUS;

            break;

        case 3:
            currentScreen_ =  DisplayScreen::SD_STATUS;

            break;

        case 4:
            currentScreen_ =  DisplayScreen::SETTINGS;

            break;

        case 5:
            currentScreen_ =  DisplayScreen::VOLTAGE;

            break;

        case 6:
            currentScreen_ =  DisplayScreen::ERROR;

            break;
    }


    DrawScreen();
}



// =====================================================
// Page Navigation
// =====================================================

void DisplayManager::NextPageItem()
{
    screenIndex_++;


    if (screenIndex_ >= PAGE_COUNT)
    {
        screenIndex_ = 0;
    }


    DrawScreen();
}


void DisplayManager::PreviousPageItem()
{
    if (screenIndex_ == 0)
    {
        screenIndex_ =
            PAGE_COUNT - 1;
    }
    else
    {
        screenIndex_--;
    }


    DrawScreen();
}


// =====================================================
// Draw Current Screen
// =====================================================

void DisplayManager::DrawScreen()
{
    oled_.Clear();


    switch (currentScreen_) 
    {
        case DisplayScreen::MAIN:

            DrawMainScreen();
            break;

        case DisplayScreen::GPS_STATUS:

            DrawGPSStatus();
            break;

        case DisplayScreen::HEADING_STATUS:

            DrawHeadingStatus();
            break;

        case DisplayScreen::SATELLITE_STATUS:

            DrawSatelliteStatus();
            break;

        case DisplayScreen::SD_STATUS:

            DrawSDStatus();
            break;

        case DisplayScreen::SETTINGS:

            DrawSettings();
            break;

        case DisplayScreen::VOLTAGE:

            DrawVoltage();
            break;

        case DisplayScreen::ERROR:

            DrawError();
            break;
    }


    oled_.Update();
}


// =====================================================
// Main Screen
// =====================================================

void DisplayManager::DrawMainScreen()
{
    oled_.DrawText(0, 0, "VERITAS TRACKER");
    oled_.DrawLine(0, 10, 127, 10);

    
    // UTC time
    char buffer[32];

    snprintf(
        buffer,
        sizeof(buffer),
        "%02d:%02d:%02d  %02d-%02d-%04d",
        position_.hour,
        position_.minute,
        position_.second,
        position_.day,
        position_.month,
        position_.year
    );

    oled_.DrawText( 0, 16, buffer);


    // -----------------------------------------
    // GPS Fix
    // -----------------------------------------

    oled_.DrawText( 0, 26, "GPS:" );

    if (position_.valid)
    {
        oled_.DrawText( 30, 26, "FIX");
    }
    else
    {
        oled_.DrawText( 30, 26, "NO ");
    }

    snprintf(
        buffer,
        sizeof(buffer),
        "SAT:%d",
        position_.satellites
    );

    oled_.DrawText(70, 26, buffer);


    // -----------------------------------------
    // HEADING
    // -----------------------------------------

    oled_.DrawText(0, 36, "HDG:" );

    if (heading_.valid)
    {
        snprintf(
            buffer,
            sizeof(buffer),
            "%.1f",
            heading_.heading
        );

        oled_.DrawText(30, 36, buffer);
    }
    else
    {
        oled_.DrawText(30, 36, "---");
    }

    
    // -----------------------------------------
    // SD Card
    // -----------------------------------------

    oled_.DrawText(0, 46, "SD :" );

    if (sd_.ready)
    {
        oled_.DrawText( 30, 46, "OK");
    }
    else
    {
        oled_.DrawText( 30, 46, "ERROR");
    }

        // Satellite
    oled_.DrawText(70, 46, "SAT:");

    if (satellite_.sending)
    {
        oled_.DrawText(100, 46, "TX");
    }
    else if (satellite_.ready)
    {
        oled_.DrawText(100, 46, "OK");
    }
    else
    {
        oled_.DrawText(100, 46, "NO");
    }


    oled_.DrawText(0, 56, "PWR: ");    
    oled_.DrawText(
        30,
        56,
        voltage_.valid
            ? VoltageMonitor::SourceName(voltage_.source)
            : "---"
    );


}

// =====================================================
// GPS Status
// =====================================================

void DisplayManager::DrawGPSStatus()
{
    oled_.DrawText(0, 0, "GPS 1 STATUS");
    oled_.DrawLine(0, 10, 127, 10);

    // -----------------------------------------
    // Fix
    // -----------------------------------------

    if (position_.valid)
    {
        oled_.DrawText(0, 16, "FIX: YES");
    }
    else
    {
        oled_.DrawText(0, 16, "FIX: NO");
    }

    // -----------------------------------------
    // Latitude
    // -----------------------------------------

    char buffer[32];

    snprintf(
        buffer,
        sizeof(buffer),
        "LAT: %.6f",
        position_.latitude
    );

    oled_.DrawText( 0, 26, buffer);


    // -----------------------------------------
    // Longitude
    // -----------------------------------------

    snprintf(
        buffer,
        sizeof(buffer),
        "LON: %.6f",
        position_.longitude
    );

    oled_.DrawText( 0, 36, buffer);


    // -----------------------------------------
    // UTC time
    // -----------------------------------------

    snprintf(
        buffer,
        sizeof(buffer),
        "UTC: %02d:%02d:%02d",
        position_.hour,
        position_.minute,
        position_.second
    );

    oled_.DrawText(
        0,
        46,
        buffer
    );


    // -----------------------------------------
    // Satellites + HDOP
    // -----------------------------------------

    snprintf(
        buffer,
        sizeof(buffer),
        "SAT:%d HDOP:%.1f",
        position_.satellites,
        position_.hdop
    ); 

    oled_.DrawText(
        0,
        56,
        buffer
    );
}


// =====================================================
// Heading Status
// =====================================================

void DisplayManager::DrawHeadingStatus()
{
    oled_.DrawText(0, 0, "HEADING");
    oled_.DrawLine(0, 10, 127, 10);

    char buffer[32];

    // Heading
    if (heading_.valid)
    {
        snprintf(
            buffer,
            sizeof(buffer),
            "HEADING: %.1f",
            heading_.heading
        );
    }
    else
    {
        snprintf(
            buffer,
            sizeof(buffer),
            "HEADING: ---"
        );
    }

    oled_.DrawText(0, 16, buffer);


    // Status
    if (heading_.valid)
    {
        oled_.DrawText(0, 28, "STATUS: OK");
    }
    else
    {
        oled_.DrawText(0, 28, "STATUS: INVALID");
    }




    oled_.DrawText(
        0,
        40,
        "SENSOR: SCX20"
    );
}


// =====================================================
// Satellite Status
// =====================================================

void DisplayManager::DrawSatelliteStatus()
{
    oled_.DrawText(0, 0, "SATELLITE");
    oled_.DrawLine(0, 10, 127, 10);

    oled_.DrawText(
        0,
        16,
        "RockBLOCK 9704"
    );

    // Current state
    if (satellite_.sending)
    {
        oled_.DrawText(
            0,
            28,
            "STATUS: SENDING"
        );
    }
    else if (satellite_.ready)
    {
        oled_.DrawText(
            0,
            28,
            "STATUS: READY"
        );
    }
    else
    {
        oled_.DrawText(
            0,
            28,
            "STATUS: OFFLINE"
        );
    }


    // Last transmission
    if (satellite_.lastTxSuccess)
    {
        oled_.DrawText(
            0,
            40,
            "LAST TX: OK"
        );
    }
    else
    {
        oled_.DrawText(
            0,
            40,
            "LAST TX: FAIL"
        );
    }


    // Signal
    char buffer[24];

    if (satellite_.signal >= 0)
    {
        snprintf(
            buffer,
            sizeof(buffer),
            "SIGNAL: %d/5",
            satellite_.signal
        );
    }
    else
    {
        snprintf(
            buffer,
            sizeof(buffer),
            "SIGNAL: ---"
        );
    }

    oled_.DrawText(
        0,
        52,
        buffer
    );
}
// =====================================================
// SD Status
// =====================================================

void DisplayManager::DrawSDStatus()
{
    oled_.DrawText(
        0,
        0,
        "SD CARD"
    );

    oled_.DrawLine(
        0,
        10,
        127,
        10
    );


    if (sd_.ready)
    {
        oled_.DrawText(
            0,
            16,
            "STATUS: OK"
        );
    }
    else
    {
        oled_.DrawText(
            0,
            16,
            "STATUS: ERROR"
        );
    }


    if (sd_.logging)
    {
        oled_.DrawText(
            0,
            28,
            "LOGGER: READY"
        );
    }
    else
    {
        oled_.DrawText(
            0,
            28,
            "LOGGER: OFF"
        );
    }
}


// =====================================================
// Settings
// =====================================================

void DisplayManager::DrawSettings()
{
    oled_.DrawText(0, 0, "SETTINGS");
    oled_.DrawLine(0, 10, 127, 10);

    oled_.DrawText(0, 16, "TX INTERVAL");

    char intervalText[12];
    if (reportingIntervalMinutes_ == 0)
    {
        snprintf(intervalText, sizeof(intervalText), "OFF");
    }
    else
    {
        snprintf(
            intervalText,
            sizeof(intervalText),
            "%u min",
            static_cast<unsigned>(reportingIntervalMinutes_)
        );
    }
    oled_.DrawText(80, 16, intervalText);

    oled_.DrawText(0, 28, "LOG INTERVAL");
    oled_.DrawText(80, 28, "1 sec");

    oled_.DrawText(0, 40, "BACK");
}

// =====================================================
// Voltage
// =====================================================

void DisplayManager::DrawVoltage()
{
    oled_.DrawText(0, 0, "POWER");
    oled_.DrawLine(0, 10, 127, 10);

    oled_.DrawText(0, 16, "Source: ");
    oled_.DrawText(50,  16,
        voltage_.valid
            ? VoltageMonitor::SourceName(voltage_.source)
            : "---"
    );

    char voltageText[16];
    oled_.DrawText(0, 26, "PSU: ");
    snprintf(
        voltageText,
        sizeof(voltageText),
        "%.2f V",
        static_cast<double>(voltage_.psu)
    );
    oled_.DrawText(30, 26, voltageText);

    oled_.DrawText(0, 36, "SYS: ");
    snprintf(
        voltageText,
        sizeof(voltageText),
        "%.2f V",
        static_cast<double>(voltage_.system)
    );
    oled_.DrawText(30, 36, voltageText);

    oled_.DrawText(0, 46, "BAT: ");
    snprintf(
        voltageText,
        sizeof(voltageText),
        "%.2f V",
        static_cast<double>(voltage_.battery)
    );
    oled_.DrawText(30, 46, voltageText);


}

// =====================================================
// Voltage
// =====================================================

void DisplayManager::DrawError()
{
    oled_.DrawText(0, 0, "ERROR");
    oled_.DrawLine(0, 10, 127, 10);


}


// =====================================================
// Get Screen
// =====================================================

DisplayScreen DisplayManager::GetScreen()
{
    return currentScreen_;
}

void DisplayManager::SetReportingIntervalMinutes(uint16_t minutes)
{
    reportingIntervalMinutes_ = minutes;
}