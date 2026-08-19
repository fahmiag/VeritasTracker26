#include "SatelliteManager.h"

SatelliteManager::SatelliteManager(
    RockBlockPlus9704& modem
)
    : modem_(modem),
      initialized_(false)
{
}

bool SatelliteManager::Begin()
{
    return modem_.IsReady();
}

void SatelliteManager::Update()
{
    if (!initialized_)
    {
        return;
    }

    modem_.Update();
}

bool SatelliteManager::SendTelemetry(
    const TelemetryPacket& packet
)
{

    Serial.println("[SAT] SendTelemetry() entered");


    if (modem_.IsSending())
    {
        Serial.println("[SAT] ERROR: RockBLOCK still sending");

        return false;
    }

    Serial.println("[SAT] RockBLOCK ready for new message");

    TelemetryPacket txPacket = packet;

   // txPacket.sequence = sequence_++;

    uint8_t buffer[32];

    uint8_t length =
        TelemetryEncoder::Encode(
            txPacket,
            buffer
        );

    if (length == 0)
    {
        Serial.println("[SAT] ERROR: encoding failed");
        return false;
    }

    char* ptr = payloadHex_;

    for (uint8_t i = 0; i < length; i++)
    {
        sprintf(
            ptr,
            "%02X",
            buffer[i]
        );

        ptr += 2;
    }

    *ptr = '\0';

    Serial.print(
        "[SAT] Payload HEX: "
    );

    Serial.println(
        payloadHex_
    );

    // return modem_.SendText(
    //     TELEMETRY_TOPIC,
    //     payloadHex_
    // );

    Serial.println();

    bool result = modem_.SendBinary(
        RAW_TOPIC, //this is important
        buffer,
        length
    );


    if (result)
    {
        Serial.println(
            "[SAT] Binary message queued"
        );
    }
    else
    {
        Serial.println(
            "[SAT] Binary message FAILED"
        );
    }

    return result;
}

bool SatelliteManager::IsBusy() const
{
    return modem_.IsSending();
}

