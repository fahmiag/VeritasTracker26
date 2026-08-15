#include "TelemetryEncoder.h"

uint8_t TelemetryEncoder::Encode(
    const TelemetryPacket& packet,
    uint8_t* buffer)
{
    uint8_t index = 0;

    buffer[index++] = packet.version;
    buffer[index++] = packet.flags;

    buffer[index++] = (packet.latitude >> 24) & 0xFF;
    buffer[index++] = (packet.latitude >> 16) & 0xFF;
    buffer[index++] = (packet.latitude >> 8) & 0xFF;
    buffer[index++] = packet.latitude & 0xFF;

    buffer[index++] = (packet.longitude >> 24) & 0xFF;
    buffer[index++] = (packet.longitude >> 16) & 0xFF;
    buffer[index++] = (packet.longitude >> 8) & 0xFF;
    buffer[index++] = packet.longitude & 0xFF;

    buffer[index++] = (packet.heading >> 8) & 0xFF;
    buffer[index++] = packet.heading & 0xFF;

    buffer[index++] = packet.satellites;

    buffer[index++] = packet.sequence >> 8;
    buffer[index++] = packet.sequence & 0xFF;

    return index;
}