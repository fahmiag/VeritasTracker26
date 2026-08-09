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

    oled_.DrawText(0, 10, "GPS TRACKER");

    oled_.DrawLine(0, 13, 127, 13);

    oled_.DrawText(0, 28, "GPS: Searching");

    oled_.DrawText(0, 40, "HDG: --.- deg");

    oled_.DrawText(0, 52, "SAT: Ready");

    oled_.DrawText(0, 63, "LOG: ON");

    oled_.Update();
}

void DisplayManager::ShowMessage(const char* message)
{
    oled_.Clear();

    oled_.DrawText(0, 20, message);

    oled_.Update();
}