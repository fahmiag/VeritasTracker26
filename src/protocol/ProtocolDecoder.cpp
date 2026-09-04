#include "ProtocolDecoder.h"

namespace Protocol
{

// ============================================================
// CRC-16/CCITT
//
// width  = 16
// poly   = 0x1021
// init   = 0x0000
// refin  = false
// refout = false
// xorout = 0x0000
//
// Check value for "123456789" = 0x31C3
// ============================================================

uint16_t ProtocolDecoder::CalculateCrc(
    const uint8_t* data,
    size_t length)
{
    if (data == nullptr)
    {
        return 0;
    }

    uint16_t crc = 0x0000;

    for (size_t i = 0; i < length; ++i)
    {
        crc ^= static_cast<uint16_t>(data[i]) << 8;

        for (uint8_t bit = 0; bit < 8; ++bit)
        {
            if (crc & 0x8000)
            {
                crc = static_cast<uint16_t>(
                    (crc << 1) ^ 0x1021
                );
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}


// ============================================================
// Decode
// ============================================================

DecodeResult ProtocolDecoder::Decode(
    const uint8_t* data,
    size_t length,
    DecodedMessage& message)
{
    // Reset output
    message.type = 0;
    message.sequence = 0;
    message.intervalMinutes = 0;

    if (data == nullptr)
    {
        return DecodeResult::NULL_BUFFER;
    }

    // Need at least:
    //
    // Version
    // Type
    // Sequence
    // Length
    // CRC
    //
    // = 7 bytes minimum

    if (length < HEADER_SIZE + CRC_SIZE)
    {
        return DecodeResult::TOO_SHORT;
    }


    // --------------------------------------------------------
    // Header
    // --------------------------------------------------------

    const uint8_t version = data[0];
    const uint8_t type    = data[1];

    // Big-endian sequence
    const uint16_t sequence =
        (static_cast<uint16_t>(data[2]) << 8) |
         static_cast<uint16_t>(data[3]);

    const uint8_t payloadLength = data[4];


    // --------------------------------------------------------
    // Version
    // --------------------------------------------------------

    if (version != VERSION)
    {
        return DecodeResult::INVALID_VERSION;
    }


    // --------------------------------------------------------
    // Message type
    // --------------------------------------------------------

    if (!IsKnownType(type))
    {
        return DecodeResult::UNKNOWN_TYPE;
    }


    // --------------------------------------------------------
    // Payload length
    // --------------------------------------------------------

    const size_t expectedLength =
        HEADER_SIZE +
        static_cast<size_t>(payloadLength) +
        CRC_SIZE;

    if (length != expectedLength)
    {
        return DecodeResult::INVALID_PACKET_LENGTH;
    }


    // --------------------------------------------------------
    // CRC
    //
    // CRC covers:
    //
    // Version
    // Type
    // Sequence
    // Length
    // Payload
    //
    // Does NOT include the CRC itself.
    // --------------------------------------------------------

    if (!ValidateCrc(data, length))
    {
        return DecodeResult::INVALID_CRC;
    }


    // --------------------------------------------------------
    // Populate common fields
    // --------------------------------------------------------

    message.type = type;
    message.sequence = sequence;


    // --------------------------------------------------------
    // Payload
    // --------------------------------------------------------

    const uint8_t* payload =
        &data[HEADER_SIZE];

    switch (type)
    {
        case TYPE_SET_INTERVAL:

            return DecodeSetInterval(
                payload,
                payloadLength,
                message
            );

        case TYPE_POSITION:
            // POSITION is currently an uplink message.
            // We don't need to decode it for OTA control.
            return DecodeResult::OK;

        case TYPE_ACK:
            // ACK decoding can be added later if required.
            return DecodeResult::OK;

        default:
            return DecodeResult::UNKNOWN_TYPE;
    }
}


// ============================================================
// SET_INTERVAL
// ============================================================

DecodeResult ProtocolDecoder::DecodeSetInterval(
    const uint8_t* payload,
    size_t payloadLength,
    DecodedMessage& message)
{
    if (payloadLength != SET_INTERVAL_PAYLOAD_SIZE)
    {
        return DecodeResult::INVALID_PAYLOAD_LENGTH;
    }

    // Big-endian uint16
    message.intervalMinutes =
        (static_cast<uint16_t>(payload[0]) << 8) |
         static_cast<uint16_t>(payload[1]);

    return DecodeResult::OK;
}


// ============================================================
// CRC validation
// ============================================================

bool ProtocolDecoder::ValidateCrc(
    const uint8_t* data,
    size_t length)
{
    if (data == nullptr || length < CRC_SIZE)
    {
        return false;
    }

    const size_t crcOffset =
        length - CRC_SIZE;

    const uint16_t calculated =
        CalculateCrc(data, crcOffset);

    // Received CRC is Big-Endian
    const uint16_t received =
        (static_cast<uint16_t>(data[crcOffset]) << 8) |
         static_cast<uint16_t>(data[crcOffset + 1]);

    return calculated == received;
}


// ============================================================
// Known message type
// ============================================================

bool ProtocolDecoder::IsKnownType(
    uint8_t type) const
{
    switch (type)
    {
        case TYPE_POSITION:
        case TYPE_ACK:
        case TYPE_SET_INTERVAL:
            return true;

        default:
            return false;
    }
}


// ============================================================
// Result string
// ============================================================

const char* ProtocolDecoder::ResultToString(
    DecodeResult result)
{
    switch (result)
    {
        case DecodeResult::OK:
            return "OK";

        case DecodeResult::NULL_BUFFER:
            return "NULL_BUFFER";

        case DecodeResult::TOO_SHORT:
            return "TOO_SHORT";

        case DecodeResult::INVALID_VERSION:
            return "INVALID_VERSION";

        case DecodeResult::UNKNOWN_TYPE:
            return "UNKNOWN_TYPE";

        case DecodeResult::INVALID_PAYLOAD_LENGTH:
            return "INVALID_PAYLOAD_LENGTH";

        case DecodeResult::INVALID_PACKET_LENGTH:
            return "INVALID_PACKET_LENGTH";

        case DecodeResult::INVALID_CRC:
            return "INVALID_CRC";

        default:
            return "UNKNOWN";
    }
}

} // namespace Protocol