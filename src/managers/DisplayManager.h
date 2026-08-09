#pragma once

#include "../drivers/OLED.h"

class DisplayManager
{
public:

    bool Begin();

    void Update();

    void ShowMainScreen();

    void ShowMessage(const char* message);

private:

    OLED oled_;
};