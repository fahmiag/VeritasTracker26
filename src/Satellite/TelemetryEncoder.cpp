#include "TelemetryEncoder.h"

uint8_t TelemetryEncoder::Encode(
    const TelemetryPacket& packet,
    uint8_t* buffer)
{
    if (buffer == nullptr)
    {
        return 0;
    }

    uint8_t index = 0;

    // ----------------------------------------
    // Version
    // ----------------------------------------

    buffer[index++] =
        packet.version;

    // ----------------------------------------
    // Type
    // ----------------------------------------

    buffer[index++] =
        packet.type;

    // ----------------------------------------
    // Flags
    // ----------------------------------------

    buffer[index++] =
        packet.flags;

    // ----------------------------------------
    // Timestamp - uint32_t
    // Big endian
    // ----------------------------------------

    buffer[index++] =
        (packet.timestamp >> 24) & 0xFF;

    buffer[index++] =
        (packet.timestamp >> 16) & 0xFF;

    buffer[index++] =
        (packet.timestamp >> 8) & 0xFF;

    buffer[index++] =
        packet.timestamp & 0xFF;

    // ----------------------------------------
    // Latitude - int32_t
    // ----------------------------------------

    buffer[index++] =
        (packet.latitude >> 24) & 0xFF;

    buffer[index++] =
        (packet.latitude >> 16) & 0xFF;

    buffer[index++] =
        (packet.latitude >> 8) & 0xFF;

    buffer[index++] =
        packet.latitude & 0xFF;

    // ----------------------------------------
    // Longitude - int32_t
    // ----------------------------------------

    buffer[index++] =
        (packet.longitude >> 24) & 0xFF;

    buffer[index++] =
        (packet.longitude >> 16) & 0xFF;

    buffer[index++] =
        (packet.longitude >> 8) & 0xFF;

    buffer[index++] =
        packet.longitude & 0xFF;

    // ----------------------------------------
    // COG - uint16_t
    // ----------------------------------------

    buffer[index++] =
        (packet.cog >> 8) & 0xFF;

    buffer[index++] =
        packet.cog & 0xFF;

    // ----------------------------------------
    // Heading - uint16_t
    // ----------------------------------------

    buffer[index++] =
        (packet.heading >> 8) & 0xFF;

    buffer[index++] =
        packet.heading & 0xFF;

    return index;
}