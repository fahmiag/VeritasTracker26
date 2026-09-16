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

    menuIndex_ = 0;

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
    uint16_t reportingIntervalMinutes
)
{
    // Store latest data
    position_  = position;
    heading_   = heading;
    sd_        = sd;
    satellite_ = satellite;

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

            if (event ==
                ButtonEvent::SELECT_SHORT)
            {
                currentScreen_ =
                    DisplayScreen::MENU;

                menuIndex_ = 0;

                DrawScreen();
            }

            break;


        // -----------------------------------------
        // MENU
        // -----------------------------------------

        case DisplayScreen::MENU:

            if (event ==
                ButtonEvent::UP_SHORT)
            {
                PreviousMenuItem();
            }

            else if (event ==
                     ButtonEvent::DOWN_SHORT)
            {
                NextMenuItem();
            }

            else if (event ==
                     ButtonEvent::SELECT_SHORT)
            {
                switch (menuIndex_)
                {
                    case 0:

                        currentScreen_ =
                            DisplayScreen::GPS_STATUS;

                        break;


                    case 1:

                        currentScreen_ =
                            DisplayScreen::HEADING_STATUS;

                        break;


                    case 2:

                        currentScreen_ =
                            DisplayScreen::SATELLITE_STATUS;

                        break;


                    case 3:

                        currentScreen_ =
                            DisplayScreen::SD_STATUS;

                        break;


                    case 4:

                        currentScreen_ =
                            DisplayScreen::SETTINGS;

                        break;
                }


                DrawScreen();
            }

            else if (
                event == ButtonEvent::BACK_SHORT ||
                event == ButtonEvent::BACK_LONG)
            {
                currentScreen_ =
                    DisplayScreen::MAIN;

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

            if (
                event == ButtonEvent::BACK_SHORT ||
                event == ButtonEvent::BACK_LONG)
            {
                currentScreen_ =
                    DisplayScreen::MENU;

                DrawScreen();
            }

            break;
    }
}


// =====================================================
// Menu Navigation
// =====================================================

void DisplayManager::NextMenuItem()
{
    menuIndex_++;


    if (menuIndex_ >= MENU_COUNT)
    {
        menuIndex_ = 0;
    }


    DrawScreen();
}


void DisplayManager::PreviousMenuItem()
{
    if (menuIndex_ == 0)
    {
        menuIndex_ =
            MENU_COUNT - 1;
    }
    else
    {
        menuIndex_--;
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


        case DisplayScreen::MENU:

            DrawMenu();

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


}

// =====================================================
// Menu
// =====================================================

void DisplayManager::DrawMenu()
{
    oled_.DrawText(
        0,
        0,
        "MENU"
    );


    oled_.DrawLine(
        0,
        10,
        127,
        10
    );


    const char* items[] =
    {
        "GPS STATUS",
        "HEADING",
        "SATELLITE",
        "SD CARD",
        "SETTINGS"
    };


    for (
        uint8_t i = 0;
        i < MENU_COUNT;
        i++)
    {
        uint8_t y =
            15 + (i * 10);


        if (i == menuIndex_)
        {
            oled_.DrawText(
                0,
                y,
                ">"
            );


            oled_.FillRect(
                10,
                y - 1,
                120,
                9,
                SSD1306_WHITE
            );


            oled_.SetTextColor(
                SSD1306_BLACK
            );


            oled_.DrawText(
                10,
                y,
                items[i]
            );


            oled_.SetTextColor(
                SSD1306_WHITE
            );
        }
        else
        {
            oled_.DrawText(
                10,
                y,
                items[i]
            );
        }
    }
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