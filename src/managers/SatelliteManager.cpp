#include "SatelliteManager.h"

// SatelliteManager::SatelliteManager(
//     RockBlockPlus9704& modem
// )
//     : modem_(modem),
//       initialized_(false),
//       sequence_(0)
// {
// }

SatelliteManager::SatelliteManager(
    RockBlockPlus9704& modem,
    ConfigurationManager& configuration)
    : modem_(modem),
      configuration_(configuration),
      initialized_(false)
{
}

bool SatelliteManager::Begin()
{
    if (!modem_.Begin())
    {
        Serial.println(
            "[SAT] RockBLOCK initialization failed"
        );

        initialized_ = false;

        return false;
    }

    initialized_ = true;

    Serial.println(
        "[SAT] Satellite manager ready"
    );

    return true;
}

void SatelliteManager::Update()
{
    if (!initialized_)
    {
        return;
    }

    modem_.Update();
    ProcessIncomingMessage();
}

bool SatelliteManager::SendTelemetry(
    const TelemetryPacket& packet
)
{

    Serial.println("[SAT] SendTelemetry() entered");

   #if DEBUG_SATELLITE_TX
        if (modem_.IsSending())
        {
            Serial.println("[SAT] ERROR: RockBLOCK still sending");

            return false;
        }
   #endif


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


    // -------------------------------------------------
    // Send binary packet
    // -------------------------------------------------
   #if DEBUG_SATELLITE_TX
        bool result = modem_.SendBinary(
            RAW_TOPIC, //this is important
            buffer,
            TelemetryEncoder::PACKET_SIZE
        );
    #else
        Serial.println("SendBinary(): Dummy");
        bool result =true;
    #endif
   
    //Debug ===============================================




    //===================================================



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

//=========================================
void SatelliteManager::ProcessIncomingMessage()
{
    if (!modem_.HasIncomingMessage())
    {
        return;
    }

    size_t receivedLength = 0;

    if (!modem_.ReceiveBinary(
            rxBuffer_,
            sizeof(rxBuffer_),
            receivedLength))
    {
        Serial.println(
            "[SAT] Failed to read MT message"
        );

        return;
    }


    Serial.printf(
        "[SAT] MT received: %u bytes\n",
        static_cast<unsigned>(receivedLength)
    );


    // --------------------------------------------------------
    // Debug HEX dump
    // --------------------------------------------------------

    Serial.print("[SAT] MT HEX: ");

    for (size_t i = 0; i < receivedLength; ++i)
    {
        if (rxBuffer_[i] < 0x10)
        {
            Serial.print('0');
        }

        Serial.print(
            rxBuffer_[i],
            HEX
        );
    }

    Serial.println();


    // --------------------------------------------------------
    // Decode
    // --------------------------------------------------------

    Protocol::DecodedMessage message;

    const Protocol::DecodeResult result =
        decoder_.Decode(
            rxBuffer_,
            receivedLength,
            message
        );

    if (result != Protocol::DecodeResult::OK)
    {
        Serial.printf(
            "[SAT] Protocol decode failed: %s\n",
            Protocol::ProtocolDecoder::ResultToString(result)
        );

        return;
    }


    Serial.printf(
        "[SAT] MT type=0x%02X seq=%u\n",
        message.type,
        message.sequence
    );


    // --------------------------------------------------------
    // Handle message
    // --------------------------------------------------------

    switch (message.type)
    {
        case Protocol::TYPE_SET_INTERVAL:

            HandleSetInterval(message);
            break;

        default:

            Serial.printf(
                "[SAT] Unsupported MT type: 0x%02X\n",
                message.type
            );

            break;
    }
}

void SatelliteManager::HandleSetInterval(
    const Protocol::DecodedMessage& message)
{
    Serial.printf(
        "[SAT] SET_INTERVAL received: %u minutes\n",
        message.intervalMinutes
    );


    const bool success =
        configuration_.SetReportingIntervalMinutes(
            message.intervalMinutes
        );


    if (success)
    {
        Serial.printf(
            "[SAT] Reporting interval changed to %u minutes\n",
            message.intervalMinutes
        );

        SendAck(
            message.sequence,
            Protocol::TYPE_SET_INTERVAL,
            Protocol::AckResult::SUCCESS,
            Protocol::AckError::NONE
        );
    }
    else
    {
        Serial.printf(
            "[SAT] Invalid reporting interval: %u\n",
            message.intervalMinutes
        );

        SendAck(
            message.sequence,
            Protocol::TYPE_SET_INTERVAL,
            Protocol::AckResult::FAILED,
            Protocol::AckError::INVALID_VALUE
        );
    }
}

bool SatelliteManager::SendAck(
    uint16_t sequence,
    uint8_t acknowledgedType,
    Protocol::AckResult result,
    Protocol::AckError error)
{
    uint8_t buffer[
        Protocol::ACK_PACKET_SIZE
    ];

    size_t encodedLength = 0;


    if (!Protocol::ProtocolEncoder::EncodeAck(
            sequence,
            acknowledgedType,
            result,
            error,
            buffer,
            sizeof(buffer),
            encodedLength))
    {
        Serial.println(
            "[SAT] Failed to encode ACK"
        );

        return false;
    }


    Serial.print("[SAT] ACK HEX: ");

    for (size_t i = 0; i < encodedLength; ++i)
    {
        if (buffer[i] < 0x10)
        {
            Serial.print('0');
        }

        Serial.print(
            buffer[i],
            HEX
        );
    }

    Serial.println();


    return modem_.SendBinary(
        RAW_TOPIC,
        buffer,
        encodedLength
    );
}