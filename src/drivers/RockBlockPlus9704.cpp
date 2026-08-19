#include "RockBlockPlus9704.h"

#include "../config/PinMap.h"

RockBlockPlus9704*
RockBlockPlus9704::instance_ = nullptr;

static rbCallbacks_t rockBlockCallbacks =
{
    nullptr,
    RockBlockPlus9704::OnMessageComplete,
    nullptr,
    nullptr,
    nullptr
};
/*  callbacks_[0] → messageProvisioning
    callbacks_[1] → moMessageComplete
    callbacks_[2] → mtMessageComplete
    callbacks_[3] → constellationState
    callbacks_[4] → moMessageStarted
*/

RockBlockPlus9704::RockBlockPlus9704()
    : serial_(1),
      initialized_(false),
      sending_(false),
      sendComplete_(false),
      sendSuccess_(false),
      lastSendSuccessful_(false)
{
}

bool RockBlockPlus9704::Begin()
{
    Serial.println("RockBLOCK: starting...");

    serial_.begin(
        BAUD_RATE,
        SERIAL_8N1,
        PinMap::ROCKBLOCK_RX,
        PinMap::ROCKBLOCK_TX
    );

    instance_ = this;

    if (!rbBegin(serial_))
    {
        Serial.println("RockBLOCK: rbBegin failed");

        initialized_ = false;

        return false;
    }

    // Register RockBLOCK library callbacks
    rbRegisterCallbacks(
        &callbacks_
    );

    initialized_ = true;

    Serial.println(
        "RockBLOCK: ready"
    );

    return true;
}

void RockBlockPlus9704::Update()
{
    if (!initialized_)
    {
        return;
    }

    rbPoll();
}

bool RockBlockPlus9704::SendText(
    uint16_t topic,
    const char* message
)
{
    if (!initialized_)
    {
        return false;
    }

    if (sending_)  //TODO: check where sending_false
    {
        Serial.println(
            "RockBLOCK: already sending"
        );

        return false;
    }

    if (message == nullptr)
    {
        return false;
    }

    Serial.println(
        "RockBLOCK: queueing message..."
    );

    bool result =
        rbSendMessageAsync(
            topic,
            message,
            strlen(message)
        );

    if (result)
    {
        sending_ = true;
        sendComplete_ = false;
        sendSuccess_ = false; 

        Serial.println(
            "RockBLOCK: message queued"
        );
    }
    else
    {
        Serial.println(
            "RockBLOCK: queue failed"
        );
    }

    return result;
}


bool RockBlockPlus9704::SendBinary(
    uint16_t topic,
    const uint8_t* data,
    size_t length
)
{
    Serial.println("RockBLOCK: SendBinary() called");

    if (!initialized_)
    {
        Serial.println("RockBLOCK: Not Initialize");
        return false;
    }

    if (sending_)
    {
        Serial.println("RockBLOCK: already sending");

        return false;
    }

    if (data == nullptr || length == 0)
    {
        Serial.println("RockBLOCK: No Data");
        return false;
    }

    Serial.println(
        "RockBLOCK: queueing binary message..."
    );

    bool result =
        rbSendMessageAsync(
            topic,
            (const char*)data,
            length
        );

    if (result)
    {
        sending_ = true;
        sendComplete_ = false;
        sendSuccess_ = false; 

        Serial.println("RockBLOCK: binary message queued");
    }
    else
    {
        Serial.println("RockBLOCK: binary queue failed" );
    }

    return result;
}

void RockBlockPlus9704::OnMessageComplete(  //TODO: Check who call this
    uint16_t id,
    rbMsgStatus_t status
)
{
    if (instance_ == nullptr)
    {
        return;
    }

    instance_->sending_ = false;
    instance_->sendComplete_ = true;

    Serial.print(
        "RockBLOCK: message complete ID="
    );

    Serial.println(id);

    if (status == RB_MSG_STATUS_OK)
    {
        instance_->sendSuccess_ = true;
        instance_->lastSendSuccessful_ = true;

        Serial.println(
            "RockBLOCK: SEND SUCCESS"
        );
    }
    else
    {
        instance_->sendSuccess_ = false;
        instance_->lastSendSuccessful_ = false;

        Serial.println(
            "RockBLOCK: SEND FAILED"
        );
    }
}

bool RockBlockPlus9704::IsReady() const
{
    return initialized_;
}


bool RockBlockPlus9704::IsSending() const
{
    return sending_;
}


bool RockBlockPlus9704::WasLastSendSuccessful() const
{
    return lastSendSuccessful_;
}
