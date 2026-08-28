#pragma once

#include <Arduino.h>
#include <TinyGPSPlus.h>

#include "../data/PositionData.h"

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

    uint32_t UTCToEpoch(
    uint16_t year,
    uint8_t month,
    uint8_t day,
    uint8_t hour,
    uint8_t minute,
    uint8_t second);
};