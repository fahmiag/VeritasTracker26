
#include "TelemetryEncoder.h"
#include <Arduino.h>

namespace
{
    // Byte 3:
    // Bits 0-1 = GPS source
    // Bits 2-7 = reserved
    constexpr uint8_t GPS_SOURCE_MASK = 0x03;

    void WriteUint16LE(
        uint8_t* buffer,
        size_t& index,
        uint16_t value)
    {
        buffer[index++] = static_cast<uint8_t>(value & 0xFF);
        buffer[index++] = static_cast<uint8_t>((value >> 8) & 0xFF);
    }

    void WriteUint32LE(
        uint8_t* buffer,
        size_t& index,
        uint32_t value)
    {
        buffer[index++] = static_cast<uint8_t>(value & 0xFF);
        buffer[index++] = static_cast<uint8_t>((value >> 8) & 0xFF);
        buffer[index++] = static_cast<uint8_t>((value >> 16) & 0xFF);
        buffer[index++] = static_cast<uint8_t>((value >> 24) & 0xFF);
    }

    void WriteInt32LE(
        uint8_t* buffer,
        size_t& index,
        int32_t value)
    {
        WriteUint32LE(
            buffer,
            index,
            static_cast<uint32_t>(value)
        );
    }
}

bool TelemetryEncoder::Encode(
    const TelemetryPacket& packet,
    uint8_t* buffer,
    size_t bufferSize)
{
    if (buffer == nullptr)
    {
        return false;
    }

    if (bufferSize < PACKET_SIZE)
    {
        return false;
    }

    size_t index = 0;

    // Byte 0: Version
    buffer[index++] = packet.version;
    Serial.println(packet.version);

    // Byte 1: Message type
    buffer[index++] = static_cast<uint8_t>(packet.type);
    Serial.println(static_cast<uint8_t>(packet.type));

    // Byte 2: Flags
    buffer[index++] = packet.flags;
     Serial.println(packet.flags);

    // Byte 3: Source
    //
    // Bits 0-1 = GPS source
    // Bits 2-7 = reserved and forced to zero
    //
    uint8_t source =
        static_cast<uint8_t>(packet.gpsSource)
        & GPS_SOURCE_MASK;

    buffer[index++] = source;
    Serial.println(source);

    // Byte 4-5: Sequence
    WriteUint16LE(
        buffer,
        index,
        packet.sequence
    );
    
    Serial.println(packet.sequence);

    // Byte 6-9: Timestamp
    WriteUint32LE(
        buffer,
        index,
        packet.timestamp
    );
    Serial.println(packet.timestamp);

    // Byte 10-13: Latitude
    WriteInt32LE(
        buffer,
        index,
        packet.latitude
    );
    
    Serial.println(packet.latitude);

    // Byte 14-17: Longitude
    WriteInt32LE(
        buffer,
        index,
        packet.longitude
    );
    Serial.println(packet.longitude);

    // Byte 18-19: COG
    WriteUint16LE(
        buffer,
        index,
        packet.cog
    );

    // Byte 20-21: Heading
    WriteUint16LE(
        buffer,
        index,
        packet.heading
    );

    return index == PACKET_SIZE;
}
/*
Byte   Size   Field
--------------------------------
0      1      Version
1      1      Type
2      1      Flags
3      1      GPS Source
4-5    2      Sequence
6-9    4      Timestamp
10-13  4      Latitude
14-17  4      Longitude
18-19  2      COG
20-21  2      Heading
--------------------------------
       22     bytes total

Scaling
--------------------------------
Latitude  = degrees × 10,000,000
Longitude = degrees × 10,000,000

COG       = degrees × 100
Heading   = degrees × 100
*/