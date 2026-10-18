#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>

#include "rockblock_9704.h"
#include "../../config/PinMap.h"

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

    bool SendBinary(
        uint16_t topic,
        const uint8_t* data,
        size_t length
    );

    bool IsReady() const;

    bool IsSending() const;

    bool WasLastSendSuccessful() const;

    void TimeOutReset();

    static rbCallbacks_t callbacks_;

    static void OnMessageComplete(
        uint16_t id,
        rbMsgStatus_t status
    );

    bool ReceiveBinary(
        uint8_t* buffer,
        size_t bufferSize,
        size_t& receivedLength
    );

    bool HasIncomingMessage() const;

    bool ConsumeSendResult(bool& success);

  

private:

    HardwareSerial serial_;

    bool initialized_;
    bool sending_;

    bool sendComplete_;
    bool sendSuccess_;
    bool lastSendSuccessful_;

    static RockBlockPlus9704* instance_;

    uint32_t sendStartTime_;

    

    static constexpr uint32_t BAUD_RATE = 230400;

    static void OnMtMessageComplete(
        const uint16_t id,
        const rbMsgStatus_t status
    );

    static bool mtMessagePending_;


};