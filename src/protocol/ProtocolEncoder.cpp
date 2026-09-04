#include "ProtocolEncoder.h"

namespace Protocol
{

// ============================================================
// CRC-16/CCITT
// ============================================================

uint16_t ProtocolEncoder::CalculateCrc(
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
// Write uint16 Big-Endian
// ============================================================

void ProtocolEncoder::WriteUint16BE(
    uint8_t* destination,
    uint16_t value)
{
    destination[0] =
        static_cast<uint8_t>(value >> 8);

    destination[1] =
        static_cast<uint8_t>(value & 0xFF);
}


// ============================================================
// Encode ACK
// ============================================================

bool ProtocolEncoder::EncodeAck(
    uint16_t sequence,
    uint8_t acknowledgedType,
    AckResult result,
    AckError error,
    uint8_t* buffer,
    size_t bufferSize,
    size_t& encodedLength)
{
    encodedLength = 0;

    if (buffer == nullptr)
    {
        return false;
    }

    if (bufferSize < ACK_PACKET_SIZE)
    {
        return false;
    }


    // --------------------------------------------------------
    // Header
    // --------------------------------------------------------

    buffer[0] = VERSION;

    buffer[1] = TYPE_ACK;

    // Sequence
    WriteUint16BE(
        &buffer[2],
        sequence
    );

    // Payload length
    buffer[4] =
        static_cast<uint8_t>(ACK_PAYLOAD_SIZE);


    // --------------------------------------------------------
    // Payload
    //
    // Byte 0 = acknowledged message type
    // Byte 1 = result
    // Byte 2 = error
    // --------------------------------------------------------

    buffer[5] =
        acknowledgedType;

    buffer[6] =
        static_cast<uint8_t>(result);

    buffer[7] =
        static_cast<uint8_t>(error);


    // --------------------------------------------------------
    // CRC
    // --------------------------------------------------------

    const uint16_t crc =
        CalculateCrc(
            buffer,
            HEADER_SIZE + ACK_PAYLOAD_SIZE
        );

    WriteUint16BE(
        &buffer[8],
        crc
    );


    encodedLength = ACK_PACKET_SIZE;

    return true;
}

} // namespace Protocol