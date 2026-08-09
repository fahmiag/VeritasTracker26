#include "DisplayManager.h"

bool DisplayManager::Begin()
{
    if (!oled_.Begin())
    {
        return false;
    }

    DrawScreen();

    return true;
}

void DisplayManager::Update(ButtonEvent event)
{
    if (event != ButtonEvent::NONE)
    {
        HandleEvent(event);
    }
}

void DisplayManager::HandleEvent(ButtonEvent event)
{
    switch (currentScreen_)
    {
        // -----------------------------------------
        // MAIN SCREEN
        // -----------------------------------------

        case DisplayScreen::MAIN:

            if (event == ButtonEvent::SELECT_SHORT)
            {
                currentScreen_ = DisplayScreen::MENU;
                menuIndex_ = 0;
                DrawScreen();
            }

            break;


        // -----------------------------------------
        // MENU
        // -----------------------------------------

        case DisplayScreen::MENU:

            if (event == ButtonEvent::UP_SHORT)
            {
                PreviousMenuItem();
            }

            else if (event == ButtonEvent::DOWN_SHORT)
            {
                NextMenuItem();
            }

            else if (event == ButtonEvent::SELECT_SHORT)
            {
                switch (menuIndex_)
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
                        currentScreen_ = DisplayScreen::SD_STATUS;
                        break;

                    case 4:
                        currentScreen_ = DisplayScreen::SETTINGS;
                        break;
                }

                DrawScreen();
            }

            else if (event == ButtonEvent::BACK_SHORT ||
                     event == ButtonEvent::BACK_LONG)
            {
                currentScreen_ = DisplayScreen::MAIN;

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

            if (event == ButtonEvent::BACK_SHORT ||
                event == ButtonEvent::BACK_LONG)
            {
                currentScreen_ = DisplayScreen::MENU;

                DrawScreen();
            }

            break;
    }
}

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
        menuIndex_ = MENU_COUNT - 1;
    }
    else
    {
        menuIndex_--;
    }

    DrawScreen();
}

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

//-----------------------------
//     Screen Display
//-----------------------------

void DisplayManager::DrawMainScreen()
{
    oled_.DrawText(0, 0, "GPS TRACKER");
    oled_.DrawLine(0, 10, 127, 10);
    oled_.DrawText(0, 16, "GPS: FIX");
    oled_.DrawText(0, 28, "HDG: 123.4");
    oled_.DrawText(0, 40, "SAT: READY");
    oled_.DrawText(0, 52, "LOG: ON");

    /*temporary value
    Later these values will come from
        GPSManager
        HeadingManager
        SatelliteManager
        LoggerManager*/
}

void DisplayManager::DrawMenu()
{
    oled_.DrawText(0, 0, "MENU");
    oled_.DrawLine(0, 10, 127, 10);

    const char* items[] =
    {
        "GPS STATUS",
        "HEADING",
        "SATELLITE",
        "SD CARD",
        "SETTINGS"
    };

    for (uint8_t i = 0; i < MENU_COUNT; i++)
    {
        uint8_t y = 15 + (i * 10); //Start row +(i * Increment)

        if (i == menuIndex_)
        {
            oled_.DrawText(0, y, ">");
        }

        oled_.DrawText(10, y, items[i]);
    }
}

void DisplayManager::DrawGPSStatus()
{
    oled_.DrawText(0, 0, "GPS STATUS");
    oled_.DrawLine(0, 10, 127, 10);
    oled_.DrawText(0, 16, "FIX: YES");
    oled_.DrawText(0, 28, "SAT: 12");
    oled_.DrawText(0, 40, "LAT: 2.9934");
    oled_.DrawText(0, 52, "LON: 101.789");
}

void DisplayManager::DrawHeadingStatus()
{
    oled_.DrawText(0, 0, "HEADING");
    oled_.DrawLine(0, 10, 127, 10);
    oled_.DrawText(0, 16, "HEADING:");
    oled_.DrawText(70, 16, "123.4");
    oled_.DrawText(0, 28, "STATUS: OK");
    oled_.DrawText(0, 40, "SCX20");
}

void DisplayManager::DrawSatelliteStatus()
{
    oled_.DrawText(0, 0, "SATELLITE");
    oled_.DrawLine(0, 10, 127, 10);
    oled_.DrawText(0, 16, "RockBLOCK");
    oled_.DrawText(0, 28, "STATUS: READY");
    oled_.DrawText(0, 40, "LAST TX: OK");
    oled_.DrawText(0, 52, "NEXT TX: 42s");
}

void DisplayManager::DrawSDStatus()
{
    oled_.DrawText(0, 0, "SD CARD");
    oled_.DrawLine(0, 10, 127, 10);
    oled_.DrawText(0, 16, "STATUS: OK");
    oled_.DrawText(0, 28, "LOGGING: ON");
    oled_.DrawText(0, 40, "FILE: 000123.CSV");
}

void DisplayManager::DrawSettings()
{
    oled_.DrawText(0, 0, "SETTINGS");
    oled_.DrawLine(0, 10, 127, 10);
    oled_.DrawText(0, 16, "TX INTERVAL");
    oled_.DrawText(80, 16, "60s");
    oled_.DrawText(0, 28, "LOG INTERVAL");
    oled_.DrawText(80, 28, "1s");
    oled_.DrawText(0, 40, "BACK");
}

DisplayScreen DisplayManager::GetScreen()
{
    return currentScreen_;
}