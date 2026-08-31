#pragma once

#include "../drivers/Button/Buttons.h"

class ButtonManager
{
public:

    bool Begin();

    void Update();

   // Button GetLastButton();
   // bool WasPressed(Button button);

    ButtonEvent GetEvent();
    bool WasEvent(ButtonEvent button);

private:

    Buttons buttons_;

    Button lastButton_ = Button::NONE;
    ButtonEvent lastEvent_ = ButtonEvent::NONE; 
};