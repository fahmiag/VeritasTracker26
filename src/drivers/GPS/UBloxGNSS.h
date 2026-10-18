#pragma once

#include <Arduino.h>
#include <TinyGPSPlus.h>

#include "../../data/PositionData.h"
#include "../../utils/DateTime.h"

class UBloxGNSS
{
public:

    UBloxGNSS();

    bool Begin();

    void Update();

    bool GetPosition(PositionData& position);

private:

    HardwareSerial serial_;

    TinyGPSPlus gps_;

    PositionData position_;

    static constexpr uint32_t BAUD_RATE = 9600;

    static constexpr uint32_t INIT_RETRY_MS = 60UL * 1000UL;

    bool uartStarted_ = false;
    uint32_t lastValidSentence_ = 0;


};