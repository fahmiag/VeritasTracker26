#!/usr/bin/env python3

import struct
from datetime import datetime, timezone,  timedelta

#test data : 
# 0101010000005EAC8A6ACDADC40117DBAB3C00000000
# 010101000000A5748A6AE8AEC40190DAAB3C00000000
# 0101010000007A738A6AE0AEC4018ED9AB3C00000000
# 0101010000004D738A6A8AAFC4019DDAAB3C00000000
PACKET_SIZE = 22

GPS_SOURCES = {
    0x00: "NONE",
    0x01: "UBLOX",
    0x02: "SCX20",
}

MESSAGE_TYPES = {
    0x01: "POSITION",
}


def decode_packet(hex_string):

    # -------------------------------------------------
    # Convert HEX string to bytes
    # -------------------------------------------------

    data = bytes.fromhex(hex_string)

    # -------------------------------------------------
    # Validate packet length
    # -------------------------------------------------

    if len(data) != PACKET_SIZE:
        raise ValueError(
            f"Invalid packet length: "
            f"expected {PACKET_SIZE} bytes, "
            f"got {len(data)}"
        )

    # -------------------------------------------------
    # Byte 0
    # -------------------------------------------------

    version = data[0]

    # -------------------------------------------------
    # Byte 1
    # -------------------------------------------------

    message_type = data[1]

    # -------------------------------------------------
    # Byte 2
    # -------------------------------------------------

    flags = data[2]

    # -------------------------------------------------
    # Byte 3
    #
    # Bits 0-1 = GPS source
    # Bits 2-7 = reserved
    # -------------------------------------------------

    source_byte = data[3]

    gps_source = source_byte & 0x03

    # -------------------------------------------------
    # Bytes 4-5
    # -------------------------------------------------

    sequence = struct.unpack_from(
        "<H",
        data,
        4
    )[0]

    # -------------------------------------------------
    # Bytes 6-9
    # -------------------------------------------------

    timestamp = struct.unpack_from(
        "<I",
        data,
        6
    )[0]

    # -------------------------------------------------
    # Bytes 10-13
    # -------------------------------------------------

    latitude_raw = struct.unpack_from(
        "<i",
        data,
        10
    )[0]

    latitude = latitude_raw / 10_000_000

    # -------------------------------------------------
    # Bytes 14-17
    # -------------------------------------------------

    longitude_raw = struct.unpack_from(
        "<i",
        data,
        14
    )[0]

    longitude = longitude_raw / 10_000_000

    # -------------------------------------------------
    # Bytes 18-19
    # -------------------------------------------------

    cog_raw = struct.unpack_from(
        "<H",
        data,
        18
    )[0]

    cog = cog_raw / 100

    # -------------------------------------------------
    # Bytes 20-21
    # -------------------------------------------------

    heading_raw = struct.unpack_from(
        "<H",
        data,
        20
    )[0]

    heading = heading_raw / 100

    # -------------------------------------------------
    # Print decoded packet
    # -------------------------------------------------

    print()
    print("================================")
    print(" Vessel Tracking Telemetry")
    print("================================")

    print(f"Packet Size     : {len(data)} bytes")
    print(f"Protocol Version: {version}")

    print(
        f"Message Type    : "
        f"{MESSAGE_TYPES.get(message_type, 'UNKNOWN')} "
        f"(0x{message_type:02X})"
    )

    print(f"Flags           : 0x{flags:02X}")

    print(
        f"GPS Source      : "
        f"{GPS_SOURCES.get(gps_source, 'UNKNOWN')} "
        f"(0x{gps_source:02X})"
    )

    print(f"Sequence        : {sequence}")
    print(f"Timestamp       : {timestamp}")

    print(f"Latitude        : {latitude:.7f}°")
    print(f"Longitude       : {longitude:.7f}°")

    print(f"COG             : {cog:.2f}°")
    print(f"Heading         : {heading:.2f}°")

    print("================================")
    # 1. Convert to a UTC datetime object
    dt_utc = datetime.fromtimestamp(timestamp, tz=timezone.utc)
    #print("UTC Datetime:", dt_utc)
    formatted_date = dt_utc.strftime('%Y-%m-%d %H:%M:%S')
    print("Formatted Date:", formatted_date)

    my_timezone = timezone(timedelta(hours=8))
    dt_malaysia = datetime.fromtimestamp(timestamp, tz=my_timezone)

    print("Malaysia Time:", dt_malaysia.strftime('%Y-%m-%d %H:%M:%S'))

    print("================================")
    print()


if __name__ == "__main__":

    hex_string = input(
        "Enter telemetry HEX: "
    ).strip()

    try:
        decode_packet(hex_string)

    except ValueError as e:
        print(f"ERROR: {e}")