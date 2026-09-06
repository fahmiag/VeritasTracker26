#!/usr/bin/env python3

import struct
from datetime import datetime, timezone, timedelta

VERSION = 1
COMMON_HEADER_SIZE = 5
CRC_SIZE = 2

MESSAGE_TYPES = {
    0x01: "POSITION",
    0x02: "ACK",
    0x80: "SET_INTERVAL",
}

ACK_RESULTS = {
    0x00: "SUCCESS",
    0x01: "FAILED",
}

ACK_ERRORS = {
    0x00: "NONE",
    0x01: "INVALID_LENGTH",
    0x02: "INVALID_VALUE",
    0x03: "UNSUPPORTED_TYPE",
    0x04: "CRC_ERROR",
    0x05: "VERSION_ERROR",
}


def crc16_ccitt(data: bytes) -> int:
    crc = 0x0000

    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF

    return crc


def decode_packet(hex_string: str):
    try:
        data = bytes.fromhex(hex_string.strip())
    except ValueError as exc:
        raise ValueError(f"Invalid HEX string: {exc}") from exc

    if len(data) < COMMON_HEADER_SIZE + CRC_SIZE:
        raise ValueError(f"Packet too short: {len(data)} bytes")

    version = data[0]
    message_type = data[1]
    sequence = struct.unpack_from(">H", data, 2)[0]
    payload_length = data[4]

    expected_length = COMMON_HEADER_SIZE + payload_length + CRC_SIZE
    if len(data) != expected_length:
        raise ValueError(
            f"Invalid packet length: header says {expected_length} bytes, "
            f"got {len(data)}"
        )

    received_crc = struct.unpack_from(">H", data, len(data) - CRC_SIZE)[0]
    calculated_crc = crc16_ccitt(data[:-CRC_SIZE])

    if received_crc != calculated_crc:
        raise ValueError(
            f"CRC mismatch: received 0x{received_crc:04X}, "
            f"calculated 0x{calculated_crc:04X}"
        )

    if version != VERSION:
        raise ValueError(f"Unsupported protocol version: {version}")

    payload = data[COMMON_HEADER_SIZE:-CRC_SIZE]

    print()
    print("================================")
    print(" Vessel Tracking Telemetry")
    print("================================")
    print(f"Packet Size     : {len(data)} bytes")
    print(f"Protocol Version: {version}")
    print(f"Message Type    : {MESSAGE_TYPES.get(message_type, 'UNKNOWN')} (0x{message_type:02X})")
    print(f"Sequence        : {sequence}")
    print(f"Payload Length  : {payload_length} bytes")
    print(f"CRC             : OK (0x{received_crc:04X})")

    if message_type == 0x01:
        decode_position(payload)
    elif message_type == 0x02:
        decode_ack(payload, sequence)
    elif message_type == 0x80:
        decode_set_interval(payload)
    else:
        print(f"Raw Payload    : {payload.hex().upper()}")

    print("================================")


def decode_position(payload: bytes):
    if len(payload) != 17:
        raise ValueError(f"POSITION payload must be 17 bytes, got {len(payload)}")

    status = payload[0]
    timestamp = struct.unpack_from(">I", payload, 1)[0]
    latitude_raw = struct.unpack_from(">i", payload, 5)[0]
    longitude_raw = struct.unpack_from(">i", payload, 9)[0]
    cog_raw = struct.unpack_from(">H", payload, 13)[0]
    heading_raw = struct.unpack_from(">H", payload, 15)[0]

    latitude = latitude_raw / 10_000_000
    longitude = longitude_raw / 10_000_000
    cog = cog_raw / 100
    heading = heading_raw / 100
    source = "SCX-20" if status & 0x10 else "u-blox"

    print("Position Status : 0x{:02X}".format(status))
    print(f"  Position Valid: {'YES' if status & 0x01 else 'NO'}")
    print(f"  COG Valid     : {'YES' if status & 0x02 else 'NO'}")
    print(f"  Heading Valid : {'YES' if status & 0x04 else 'NO'}")
    print(f"  Time Valid    : {'YES' if status & 0x08 else 'NO'}")
    print(f"  Position Source: {source}")
    print(f"Timestamp       : {timestamp}")

    dt_utc = datetime.fromtimestamp(timestamp, tz=timezone.utc)
    dt_malaysia = dt_utc.astimezone(timezone(timedelta(hours=8)))
    print(f"UTC Datetime    : {dt_utc:%Y-%m-%d %H:%M:%S}")
    print(f"Malaysia Time   : {dt_malaysia:%Y-%m-%d %H:%M:%S}")

    print(f"Latitude        : {latitude:.7f}°")
    print(f"Longitude       : {longitude:.7f}°")
    print(f"COG             : {cog:.2f}°")
    print(f"Heading         : {heading:.2f}°")


def decode_set_interval(payload: bytes):
    if len(payload) != 2:
        raise ValueError(f"SET_INTERVAL payload must be 2 bytes, got {len(payload)}")

    minutes = struct.unpack(">H", payload)[0]
    state = "OFF" if minutes == 0 else f"every {minutes} minute(s)"

    print(f"Reporting Interval: {minutes} minute(s) ({state})")


def decode_ack(payload: bytes, sequence: int):
    if len(payload) != 3:
        raise ValueError(f"ACK payload must be 3 bytes, got {len(payload)}")

    acknowledged_type = payload[0]
    result = payload[1]
    error = payload[2]

    print(f"ACK Sequence    : {sequence}")
    print(f"Acknowledged Type: {MESSAGE_TYPES.get(acknowledged_type, 'UNKNOWN')} (0x{acknowledged_type:02X})")
    print(f"Result          : {ACK_RESULTS.get(result, 'UNKNOWN')} (0x{result:02X})")
    print(f"Error           : {ACK_ERRORS.get(error, 'UNKNOWN')} (0x{error:02X})")


if __name__ == "__main__":
    hex_string = input("Enter telemetry HEX: ").strip()

    try:
        decode_packet(hex_string)
    except ValueError as exc:
        print(f"ERROR: {exc}")
