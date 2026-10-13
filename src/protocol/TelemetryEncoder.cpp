#include "TelemetryEncoder.h"

namespace
{
    void WriteUint16BE(uint8_t* buffer, size_t& index, uint16_t value)
    {
        buffer[index++] = static_cast<uint8_t>((value >> 8) & 0xFF);
        buffer[index++] = static_cast<uint8_t>(value & 0xFF);
    }

    void WriteUint32BE(uint8_t* buffer, size_t& index, uint32_t value)
    {
        buffer[index++] = static_cast<uint8_t>((value >> 24) & 0xFF);
        buffer[index++] = static_cast<uint8_t>((value >> 16) & 0xFF);
        buffer[index++] = static_cast<uint8_t>((value >> 8) & 0xFF);
        buffer[index++] = static_cast<uint8_t>(value & 0xFF);
    }

    void WriteInt32BE(uint8_t* buffer, size_t& index, int32_t value)
    {
        WriteUint32BE(buffer, index, static_cast<uint32_t>(value));
    }
}

uint16_t TelemetryEncoder::Crc16(
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
                crc = static_cast<uint16_t>((crc << 1) ^ 0x1021);
            }
            else
            {
                crc = static_cast<uint16_t>(crc << 1);
            }
        }
    }

    return crc;
}

bool TelemetryEncoder::Encode(
    const TelemetryPacket& packet,
    uint8_t* buffer,
    size_t bufferSize)
{
    if (buffer == nullptr || bufferSize < PACKET_SIZE)
    {
        return false;
    }

    if (packet.type != Telemetry::Type::POSITION&&
        packet.type != Telemetry::Type::EMERGENCY &&
        packet.type != Telemetry::Type::TAMPER &&
        packet.type != Telemetry::Type::POWER_LOSS)
    {
        return false;
    }

    size_t index = 0;

    // Common header
    buffer[index++] = packet.version;
    buffer[index++] = static_cast<uint8_t>(packet.type);
    WriteUint16BE(buffer, index, packet.sequence);
    buffer[index++] = static_cast<uint8_t>(Telemetry::POSITION_PAYLOAD_SIZE);

    // POSITION payload
    uint8_t status = packet.status & ~Telemetry::RESERVED_MASK;

    if (packet.positionSource == Telemetry::PositionSource::SCX20)
    {
        status |= Telemetry::SOURCE_SCX20;
    }
    else
    {
        status &= static_cast<uint8_t>(~Telemetry::SOURCE_SCX20);
    }

    buffer[index++] = status;
    WriteUint32BE(buffer, index, packet.timestamp);
    WriteInt32BE(buffer, index, packet.latitude);
    WriteInt32BE(buffer, index, packet.longitude);
    WriteUint16BE(buffer, index, packet.cog);
    WriteUint16BE(buffer, index, packet.heading);
    WriteUint16BE(buffer, index, packet.systemVoltageMv);

    // CRC covers Version through Payload.
    const uint16_t crc = Crc16(buffer, index);
    WriteUint16BE(buffer, index, crc);

    return index == PACKET_SIZE;
}
