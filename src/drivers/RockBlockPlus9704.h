#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>

#include "rockblock_9704.h"

class RockBlockPlus9704
{
public:

    RockBlockPlus9704();

    bool Begin();

    void Update();

    bool SendText(
        uint16_t topic,
        const char* message
    );

    bool IsReady() const;

    bool IsSending() const;

    bool WasLastSendSuccessful() const;

    bool SendBinary(
        uint16_t topic,
        const uint8_t* data,
        uint16_t length
    );

private:

    HardwareSerial serial_;

    bool initialized_;
    bool sending_;

    bool sendComplete_;
    bool sendSuccess_;
    bool lastSendSuccessful_;

    static RockBlockPlus9704* instance_;

    static void OnMessageComplete(
        uint16_t id,
        rbMsgStatus_t status
    );

    static constexpr uint32_t BAUD_RATE = 230400;
};