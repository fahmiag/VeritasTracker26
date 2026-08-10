#pragma once

#include <Arduino.h>

class StatusLED
{
public:
    enum class Mode 
    { 
        OFF, 
        NORMAL, 
        ERROR 
    };

    bool Begin();
    void Update();

    void SetMode(Mode mode); 
    Mode GetMode() const;

private:

    Mode mode_ = Mode::OFF;
    bool state_ = false;

    uint32_t lastToggleTime_ = 0;


};