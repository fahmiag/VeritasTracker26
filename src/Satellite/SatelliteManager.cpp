#include "SatelliteManager.h"

SatelliteManager::SatelliteManager(
    RockBlockPlus9704& modem
)
    : modem_(modem),
      sequence_(0)
{
}

bool SatelliteManager::Begin()
{
    return modem_.IsReady();
}

void SatelliteManager::Update()
{
}

bool SatelliteManager::SendTelemetry(
    const TelemetryPacket& packet
)
{
    if (modem_.IsSending())
    {
        return false;
    }

    TelemetryPacket txPacket = packet;

    txPacket.sequence = sequence_++;

    uint8_t buffer[32];

    uint8_t length =
        TelemetryEncoder::Encode(
            txPacket,
            buffer
        );

    if (length == 0)
    {
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

    return modem_.SendBinary(
        RAW_TOPIC, //this is important
        buffer,
        length
    );
}

bool SatelliteManager::IsBusy() const
{
    return modem_.IsSending();
}

