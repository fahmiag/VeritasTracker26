#include "DisplayManager.h"

bool DisplayManager::Begin()
{
    return oled_.Begin();
}

void DisplayManager::Update()
{
    oled_.Update();
}

void DisplayManager::ShowMainScreen()
{
    oled_.Clear();

    oled_.DrawText(0, 0, "GPS TRACKER");

    oled_.DrawLine(0, 10, 127, 10);

    oled_.DrawText(0, 15, "GPS: Searching");

    oled_.DrawText(0, 26, "HDG: --.- deg");

    oled_.DrawText(0, 37, "SAT: Ready");

    oled_.DrawText(0, 48, "LOG: ON");

    oled_.Update();
}

void DisplayManager::ShowMessage(const char* message)
{
    oled_.Clear();

    oled_.DrawText(0, 20, message);

    oled_.Update();
}