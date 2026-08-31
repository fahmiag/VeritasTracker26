#include "Buttons.h"

#include "../../config/PinMap.h"
#include "../../config/Config.h"


bool Buttons::Begin()
{
    buttons_[0] = 
    { 
        PinMap::BUTTON_UP, 
        Button::UP, 
        HIGH, 
        HIGH, 
        0,
        0,
        false
    }; 

    buttons_[1] = 
    { 
        PinMap::BUTTON_DOWN, 
        Button::DOWN, 
        HIGH, 
        HIGH, 
        0,
        0,
        false 
    }; 
    
    buttons_[2] = 
    { 
        PinMap::BUTTON_SELECT, 
        Button::SELECT, 
        HIGH, 
        HIGH, 
        0,
        0,
        false
    }; 
    buttons_[3] = 
    { 
        PinMap::BUTTON_BACK, 
        Button::BACK, 
        HIGH, 
        HIGH, 
        0,
        0,
        false
    }; 

    for (auto& button : buttons_) 
    { 
        pinMode(button.pin, INPUT_PULLUP); 
        button.lastReading = digitalRead(button.pin); 
        button.stableState = button.lastReading; 
        button.lastChangeTime = millis(); 
        button.pressStartTime = 0; 
        button.longPressReported = false;
    }


    return true;
}

ButtonEvent Buttons::Update() 
{ 
    uint32_t now = millis(); 
    
    for (auto& button : buttons_) //check all button
    { 
        bool reading = digitalRead(button.pin); 
        
        // ------------------------------------------------- 
        // Raw signal changed 
        // -------------------------------------------------

        if (reading != button.lastReading) 
        { 
            button.lastChangeTime = now; 
            button.lastReading = reading; 
        } 

        // ------------------------------------------------- 
        // Wait for debounce 
        // -------------------------------------------------

        
        if ((now - button.lastChangeTime) < 
            Config::BUTTON_DEBOUNCE_MS) 
        {
            continue;
        }
        
        // ------------------------------------------------- 
        // Stable state changed 
        // -------------------------------------------------       
        if (reading != button.stableState) 
        { 
            button.stableState = reading; 

            // --------------------------------------------- 
            // BUTTON PRESSED 
            // ---------------------------------------------
            
            // INPUT_PULLUP: 
            // LOW = button pressed 
            if (button.stableState == LOW) 
            { 
                button.pressStartTime = now; 
                button.longPressReported = false;
            } 

            // --------------------------------------------- 
            // BUTTON RELEASED 
            // --------------------------------------------- 
            else 
            { 
                uint32_t pressDuration = now - button.pressStartTime; 

                // Short press 
                if (!button.longPressReported) 
                { 
                    return MakeEvent( 
                        button.button, 
                        false 
                    ); 
                } 
            }

        }    

        // ------------------------------------------------- 
        // Check for long press 
        // ------------------------------------------------- 
        if (button.stableState == LOW && !button.longPressReported) 
        { 
            uint32_t pressDuration = now - button.pressStartTime; 

            if (pressDuration >= Config::BUTTON_LONG_PRESS_MS) 
            { 
                button.longPressReported = true; 

                return MakeEvent( 
                    button.button, 
                    true 
                );
            } 
        }
    } 

    return ButtonEvent::NONE; 
}

ButtonEvent Buttons::MakeEvent( Button button, bool longPress) 
{ 
    switch (button) 
    { 
    case Button::UP: 
        return longPress 
            ? ButtonEvent::UP_LONG 
            : ButtonEvent::UP_SHORT; 
            
    case Button::DOWN: 
        return longPress 
        ? ButtonEvent::DOWN_LONG 
        : ButtonEvent::DOWN_SHORT; 
        
    case Button::SELECT: 
        return longPress 
        ? ButtonEvent::SELECT_LONG 
        : ButtonEvent::SELECT_SHORT; 

    case Button::BACK: 
        return longPress 
        ? ButtonEvent::BACK_LONG 
        : ButtonEvent::BACK_SHORT; 

    default: 
        return ButtonEvent::NONE; 
    } 
}

