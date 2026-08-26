#include "SatelliteManager.h"

SatelliteManager::SatelliteManager(
    RockBlockPlus9704& modem
)
    : modem_(modem),
      initialized_(false),
      sequence_(0)
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

    txPacket.sequence = sequence_++;
    Serial.print("[SAT] Sequence: ");
    Serial.println(txPacket.sequence);

    uint8_t buffer[TelemetryEncoder::PACKET_SIZE];


    bool encoded =
        TelemetryEncoder::Encode(
            txPacket,
            buffer,
            sizeof(buffer)
        );

    if (!encoded)
    {
        Serial.println("[SAT] ERROR: encoding failed");
        return false;
    }

    // -------------------------------------------------
    // Convert binary payload to HEX for debugging
    // -------------------------------------------------

    char* ptr = payloadHex_;

    for (uint8_t i = 0; i < TelemetryEncoder::PACKET_SIZE; i++)
    {
        sprintf(
            ptr,
            "%02X",
            buffer[i]
        );

        ptr += 2;
    }

    *ptr = '\0';

    Serial.print("[SAT] Payload HEX: ");
    Serial.println(payloadHex_);
    Serial.println();

    // return modem_.SendText(
    //     TELEMETRY_TOPIC,
    //     payloadHex_
    // );

    // -------------------------------------------------
    // Send binary packet
    // -------------------------------------------------

   

    bool result = modem_.SendBinary(
        RAW_TOPIC, //this is important
        buffer,
        TelemetryEncoder::PACKET_SIZE
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

