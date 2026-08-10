#include "StatusLED.h"

#include "../config/PinMap.h"
#include "../config/Config.h"


bool StatusLED::Begin()
{
    pinMode(PinMap::LED_STATUS, OUTPUT);

    state_ = false;

    digitalWrite(
        PinMap::LED_STATUS,
        LOW
    );

    lastToggleTime_ = millis();

    return true;
}


void StatusLED::Update()
{
    uint32_t now = millis();
    uint32_t interval;

    switch (mode_) 
    { 
        case Mode::NORMAL: 
            
            if (state_)
            {
                interval = Config::NORMAL_ON_MS;
            }
            else
            {
                interval = Config::NORMAL_OFF_MS;
            }
            break; 

        case Mode::ERROR: 
            interval = Config::ERROR_INTERVAL_MS; 
            break; 
        case Mode::OFF: 
        default: 
            digitalWrite( PinMap::LED_STATUS, LOW ); 
            state_ = false; 
        return; 
    }




    if ((now - lastToggleTime_) >= interval)
    {
        lastToggleTime_ = now;

        state_ = !state_;

        digitalWrite(
            PinMap::LED_STATUS,
            state_ ? HIGH : LOW
        );
    }
}

void StatusLED::SetMode(Mode mode)
{
    mode_ = mode;

    lastToggleTime_ = millis();

    // Reset LED when changing mode
    state_ = false;

    digitalWrite(
        PinMap::LED_STATUS,
        LOW
    );

}

StatusLED::Mode StatusLED::GetMode() const
{
    return mode_;
}