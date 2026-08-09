#pragma once

#include "../drivers/Buttons.h"

class ButtonManager
{
public:

    bool Begin();

    void Update();

    Button GetLastButton();

    bool WasPressed(Button button);

private:

    Buttons buttons_;

    Button lastButton_ = Button::NONE;
};