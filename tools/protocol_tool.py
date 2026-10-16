#!/usr/bin/env python3

import argparse
import base64
import binascii
import sys
import struct
from datetime import datetime, timezone


# ============================================================
# Protocol constants
# ============================================================

# Control messages remain V1; navigation messages support V1 and V2.
PROTOCOL_VERSION = 0x01  # Compatibility alias for existing packet builders.
CONTROL_VERSION = 0x01
NAVIGATION_VERSION = 0x02
VOLTAGE_UNAVAILABLE = 0xFFFF
HEADING_INVALID = 40000  # 400.00 degrees, with HEADING_VALID clear.

TYPE_POSITION = 0x01
TYPE_ACK = 0x02
TYPE_EMERGENCY = 0x03
TYPE_TAMPER = 0x04
TYPE_POWER_LOSS = 0x05
TYPE_SET_INTERVAL = 0x80

HEADER_SIZE = 5
CRC_SIZE = 2

POSITION_PAYLOAD_SIZE = 17  # Legacy V1 payload size.
NAVIGATION_PAYLOAD_SIZES = {1: 17, 2: 19}
ACK_PAYLOAD_SIZE = 3
SET_INTERVAL_PAYLOAD_SIZE = 2


MESSAGE_TYPE_NAMES = {
    TYPE_POSITION: "POSITION",
    TYPE_ACK: "ACK",
    TYPE_EMERGENCY: "EMERGENCY",
    TYPE_TAMPER: "TAMPER",
    TYPE_POWER_LOSS: "POWER_LOSS",
    TYPE_SET_INTERVAL: "SET_INTERVAL",
}


NAVIGATION_TYPES = (
    TYPE_POSITION, TYPE_EMERGENCY, TYPE_TAMPER, TYPE_POWER_LOSS
)

PAYLOAD_SIZES = {
    **{message_type: POSITION_PAYLOAD_SIZE for message_type in NAVIGATION_TYPES},
    TYPE_ACK: ACK_PAYLOAD_SIZE,
    TYPE_SET_INTERVAL: SET_INTERVAL_PAYLOAD_SIZE,
}

# Wire ACK values from Protocol::AckError, used by ProtocolEncoder.
ACK_RESULT_NAMES = {0x00: "SUCCESS", 0x01: "FAILED"}
ACK_ERROR_NAMES = {
    0x00: "NONE",
    0x01: "INVALID_COMMAND",
    0x02: "INVALID_VALUE",
    0x03: "UNSUPPORTED_VERSION",
    0x04: "INVALID_CRC",
    0x05: "INTERNAL_ERROR",
}

STATUS_POSITION_VALID = 0x01
STATUS_COG_VALID = 0x02
STATUS_HEADING_VALID = 0x04
STATUS_TIME_VALID = 0x08
STATUS_SOURCE_SCX20 = 0x10
STATUS_EMERGENCY = 0x20
STATUS_TAMPER = 0x40
STATUS_PSU_POWER = 0x80


# ============================================================
# CRC-16
#
# width  = 16
# poly   = 0x1021
# init   = 0x0000
# refin  = false
# refout = false
# xorout = 0x0000
#
# check("123456789") = 0x31C3
# ============================================================

def calculate_crc(data: bytes) -> int:
    crc = 0x0000

    for byte in data:
        crc ^= byte << 8

        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF

    return crc


# ============================================================
# Helpers
# ============================================================

def write_uint16_be(value: int) -> bytes:
    return value.to_bytes(2, byteorder="big", signed=False)


def read_uint16_be(data: bytes) -> int:
    return int.from_bytes(data, byteorder="big", signed=False)


def read_uint32_be(data: bytes) -> int:
    return int.from_bytes(data, byteorder="big", signed=False)


def read_int32_be(data: bytes) -> int:
    return int.from_bytes(data, byteorder="big", signed=True)


def bytes_to_hex(data: bytes) -> str:
    return data.hex().upper()


def bytes_to_base64(data: bytes) -> str:
    return base64.b64encode(data).decode("ascii")


# ============================================================
# Generic packet builder
# ============================================================

def build_packet(
    message_type: int,
    sequence: int,
    payload: bytes,
    version: int = PROTOCOL_VERSION
) -> bytes:

    if not 0 <= version <= 0xFF:
        raise ValueError("Version must be between 0 and 255")

    if not 0 <= message_type <= 0xFF:
        raise ValueError("Message type must be between 0 and 255")

    if not 0 <= sequence <= 0xFFFF:
        raise ValueError("Sequence must be between 0 and 65535")

    if len(payload) > 0xFF:
        raise ValueError("Payload too large")

    packet = bytearray()

    # Header
    packet.append(version)
    packet.append(message_type)

    packet.extend(
        write_uint16_be(sequence)
    )

    packet.append(len(payload))

    # Payload
    packet.extend(payload)

    # CRC
    crc = calculate_crc(packet)

    packet.extend(
        write_uint16_be(crc)
    )

    return bytes(packet)


def build_navigation(
    message_type: int,
    sequence: int,
    *,
    status: int = 0,
    timestamp: int = 0,
    latitude_raw: int = 0,
    longitude_raw: int = 0,
    cog_raw: int = 0,
    heading_raw: int = HEADING_INVALID,
    system_voltage_mv: int = VOLTAGE_UNAVAILABLE,
    version: int = NAVIGATION_VERSION
) -> bytes:
    """Build navigation test packets using raw wire units and explicit flags.

    Coordinates use degrees * 10,000,000; angles use degrees * 100.
    The source bit is supplied in status; the firmware encoder derives it
    from positionSource. Values are not converted from floating point here.
    """
    if message_type not in NAVIGATION_TYPES:
        raise ValueError("Type must be POSITION, EMERGENCY, TAMPER or POWER_LOSS")
    if version not in NAVIGATION_PAYLOAD_SIZES:
        raise ValueError("Navigation version must be 1 or 2")
    fields = (
        ("Status", status, 0, 255),
        ("Timestamp", timestamp, 0, 0xFFFFFFFF),
        ("Latitude raw", latitude_raw, -0x80000000, 0x7FFFFFFF),
        ("Longitude raw", longitude_raw, -0x80000000, 0x7FFFFFFF),
        ("COG raw", cog_raw, 0, 65535),
        ("Heading raw", heading_raw, 0, 65535),
        ("System voltage mV", system_voltage_mv, 0, 65535),
    )
    for name, value, minimum, maximum in fields:
        if not isinstance(value, int) or not minimum <= value <= maximum:
            raise ValueError(f"{name} must be an integer between {minimum} and {maximum}")
    if version == 1 and system_voltage_mv != VOLTAGE_UNAVAILABLE:
        raise ValueError("Version 1 has no system voltage field")
    payload = struct.pack(
        ">BIiiHH", status, timestamp, latitude_raw, longitude_raw, cog_raw, heading_raw
    )
    if version == NAVIGATION_VERSION:
        payload += write_uint16_be(system_voltage_mv)
    return build_packet(message_type, sequence, payload, version=version)


# ============================================================
# SET_INTERVAL
# ============================================================

def build_set_interval(
    sequence: int,
    interval_minutes: int
) -> bytes:

    if not 0 <= interval_minutes <= 1440:
        raise ValueError(
            "Interval must be between 0 and 1440 minutes"
        )

    payload = write_uint16_be(
        interval_minutes
    )

    return build_packet(
        TYPE_SET_INTERVAL,
        sequence,
        payload,
        version=CONTROL_VERSION
    )


# ============================================================
# ACK
# ============================================================

def build_ack(
    sequence: int,
    acknowledged_type: int,
    result: int,
    error: int
) -> bytes:

    if not 0 <= acknowledged_type <= 0xFF:
        raise ValueError("Acknowledged type must be between 0 and 255")
    if result not in ACK_RESULT_NAMES:
        raise ValueError("ACK result must be SUCCESS (0) or FAILED (1)")
    if error not in ACK_ERROR_NAMES:
        raise ValueError("ACK error must be between 0 and 5")

    payload = bytes([
        acknowledged_type,
        result,
        error
    ])

    return build_packet(
        TYPE_ACK,
        sequence,
        payload,
        version=CONTROL_VERSION
    )


# ============================================================
# Packet validation / decoding
# ============================================================

def decode_packet(packet: bytes):

    print()
    print("========================================")
    print(" Vessel Tracker Protocol Decoder")
    print("========================================")

    print(f"Packet Size     : {len(packet)} bytes")
    print(f"Raw HEX         : {bytes_to_hex(packet)}")
    print(f"Base64          : {bytes_to_base64(packet)}")

    if len(packet) < HEADER_SIZE + CRC_SIZE:
        print()
        print("ERROR: Packet too short")
        return False

    # --------------------------------------------------------
    # Header
    # --------------------------------------------------------

    version = packet[0]
    message_type = packet[1]

    sequence = read_uint16_be(
        packet[2:4]
    )

    payload_length = packet[4]

    expected_length = (
        HEADER_SIZE +
        payload_length +
        CRC_SIZE
    )

    print()
    print(f"Protocol Version: {version}")

    type_name = MESSAGE_TYPE_NAMES.get(
        message_type,
        "UNKNOWN"
    )

    print(
        f"Message Type    : "
        f"{type_name} (0x{message_type:02X})"
    )

    print(f"Sequence        : {sequence}")
    print(f"Payload Length  : {payload_length} bytes")

    # --------------------------------------------------------
    # Version
    # --------------------------------------------------------

    if message_type in NAVIGATION_TYPES:
        required_payload_size = NAVIGATION_PAYLOAD_SIZES.get(version)
    elif message_type in (TYPE_ACK, TYPE_SET_INTERVAL):
        required_payload_size = PAYLOAD_SIZES[message_type] if version == CONTROL_VERSION else None
    else:
        print(f"ERROR: Unsupported message type 0x{message_type:02X}")
        return False

    if required_payload_size is None:
        print(f"ERROR: Unsupported protocol version {version} for {type_name}")
        return False

    # --------------------------------------------------------
    # Length
    # --------------------------------------------------------

    if len(packet) != expected_length:
        print()
        print(
            f"ERROR: Invalid packet length. "
            f"Expected {expected_length}, "
            f"got {len(packet)}"
        )

        return False

    # --------------------------------------------------------
    # CRC
    # --------------------------------------------------------

    received_crc = read_uint16_be(
        packet[-2:]
    )

    calculated_crc = calculate_crc(
        packet[:-2]
    )

    crc_ok = received_crc == calculated_crc

    if crc_ok:
        print(
            f"CRC             : OK "
            f"(0x{received_crc:04X})"
        )
    else:
        print(
            f"CRC             : FAILED"
        )

        print(
            f"  Received      : 0x{received_crc:04X}"
        )

        print(
            f"  Calculated    : 0x{calculated_crc:04X}"
        )

        return False

    payload = packet[
        HEADER_SIZE:
        HEADER_SIZE + payload_length
    ]

    # --------------------------------------------------------
    # Message-specific decode
    # --------------------------------------------------------

    if payload_length != required_payload_size:
        print(
            f"ERROR: {type_name} payload must be "
            f"{required_payload_size} bytes, got {payload_length}"
        )
        return False

    if message_type == TYPE_SET_INTERVAL:
        return decode_set_interval(payload)
    if message_type == TYPE_ACK:
        return decode_ack(payload)
    return decode_position(payload, message_type, version)


# ============================================================
# Decode SET_INTERVAL
# ============================================================

def decode_set_interval(payload: bytes):

    print()
    print("----------------------------------------")
    print(" SET_INTERVAL")
    print("----------------------------------------")

    if len(payload) != SET_INTERVAL_PAYLOAD_SIZE:
        print(
            f"ERROR: SET_INTERVAL payload must be "
            f"{SET_INTERVAL_PAYLOAD_SIZE} bytes"
        )

        return False

    interval_minutes = read_uint16_be(
        payload
    )

    print(
        f"Reporting Interval: "
        f"{interval_minutes} minutes"
    )

    if interval_minutes == 0:
        print("Telemetry         : OFF")
    elif interval_minutes <= 1440:
        print("Telemetry         : ENABLED")
    else:
        print("Telemetry         : INVALID VALUE")
        return False

    return True


# ============================================================
# Decode ACK
# ============================================================

def decode_ack(payload: bytes):

    print()
    print("----------------------------------------")
    print(" ACK")
    print("----------------------------------------")

    if len(payload) != ACK_PAYLOAD_SIZE:
        print(
            f"ERROR: ACK payload must be "
            f"{ACK_PAYLOAD_SIZE} bytes"
        )

        return False

    acknowledged_type = payload[0]
    result = payload[1]
    error = payload[2]

    acknowledged_name = MESSAGE_TYPE_NAMES.get(
        acknowledged_type,
        "UNKNOWN"
    )

    print(
        f"Acknowledged Type: "
        f"{acknowledged_name} "
        f"(0x{acknowledged_type:02X})"
    )

    print(
        f"Result           : "
        f"{ACK_RESULT_NAMES.get(result, 'UNKNOWN')} "
        f"(0x{result:02X})"
    )

    print(
        f"Error Code       : "
        f"{ACK_ERROR_NAMES.get(error, 'UNKNOWN')} (0x{error:02X})"
    )

    if result not in ACK_RESULT_NAMES or error not in ACK_ERROR_NAMES:
        print("ERROR: Unsupported ACK result or error code")
        return False

    return True


# ============================================================
# Decode POSITION
# ============================================================

def decode_position(
    payload: bytes, message_type: int = TYPE_POSITION, version: int = PROTOCOL_VERSION
):

    print()
    print("----------------------------------------")
    print(f" {MESSAGE_TYPE_NAMES[message_type]}")
    print("----------------------------------------")

    expected_size = NAVIGATION_PAYLOAD_SIZES.get(version)
    if expected_size is None:
        print(f"ERROR: Unsupported navigation version {version}")
        return False
    if len(payload) != expected_size:
        print(
            f"ERROR: {MESSAGE_TYPE_NAMES[message_type]} V{version} payload must be "
            f"{expected_size} bytes"
        )

        return False

    position_status = payload[0]

    timestamp = read_uint32_be(
        payload[1:5]
    )

    latitude_raw = read_int32_be(
        payload[5:9]
    )

    longitude_raw = read_int32_be(
        payload[9:13]
    )

    cog_raw = read_uint16_be(
        payload[13:15]
    )

    heading_raw = read_uint16_be(
        payload[15:17]
    )

    position_valid = bool(
        position_status & STATUS_POSITION_VALID
    )

    cog_valid = bool(
        position_status & STATUS_COG_VALID
    )

    heading_valid = bool(
        position_status & STATUS_HEADING_VALID
    )

    time_valid = bool(
        position_status & STATUS_TIME_VALID
    )

    source_scx20 = bool(
        position_status & STATUS_SOURCE_SCX20
    )

    source = (
        "SCX-20"
        if source_scx20
        else "u-blox"
    )

    emergency = bool(position_status & STATUS_EMERGENCY)
    tamper = bool(position_status & STATUS_TAMPER)
    powered_by_psu = bool(position_status & STATUS_PSU_POWER)

    latitude = latitude_raw / 10_000_000.0
    longitude = longitude_raw / 10_000_000.0

    cog = cog_raw / 100.0
    heading = heading_raw / 100.0

    print(
        f"Position Status : "
        f"0x{position_status:02X}"
    )

    print(
        f"  Position Valid: "
        f"{'YES' if position_valid else 'NO'}"
    )

    print(
        f"  COG Valid     : "
        f"{'YES' if cog_valid else 'NO'}"
    )

    print(
        f"  Heading Valid : "
        f"{'YES' if heading_valid else 'NO'}"
    )

    print(
        f"  Time Valid    : "
        f"{'YES' if time_valid else 'NO'}"
    )

    print(
        f"  Position Source: {source}"
    )

    print()

    #print(f"Emergency Latch : {'ACTIVE' if emergency else 'CLEAR'}")
    #print(f"Tamper Latch    : {'ACTIVE' if tamper else 'CLEAR'}")
    print(
        f"Panic GPIO      : "
        f"{'HIGH (1), released' if emergency else 'LOW (0), pressed'}"
    )
    print(
        f"Tamper GPIO     : "
        f"{'HIGH (1), enclosure open' if tamper else 'LOW (0), enclosure closed'}"
    )

    print(f"PSU Power       : {'YES' if powered_by_psu else 'NO (battery/USB/unknown)'}")
    print(f"Timestamp       : {timestamp}")

    if not time_valid:
        print("UTC Time        : INVALID (TIME_VALID=0)")
    else:
        try:
            utc_time = datetime.fromtimestamp(timestamp, tz=timezone.utc)
            print(f"UTC Time        : {utc_time.isoformat()}")
        except (ValueError, OSError, OverflowError):
            print("UTC Time        : Invalid timestamp")

    print(
        f"Latitude        : {latitude:.7f}°"
        f"{'' if position_valid else ' (INVALID)'}"
    )

    print(
        f"Longitude       : {longitude:.7f}°"
        f"{'' if position_valid else ' (INVALID)'}"
    )

    print(
        f"COG             : {cog:.2f}°"
        f"{'' if cog_valid else ' (INVALID)'}"
    )

    print(
        f"Heading         : {heading:.2f}°"
        f"{'' if heading_valid else ' (INVALID)'}"
    )

    if version == NAVIGATION_VERSION:
        voltage_mv = read_uint16_be(payload[17:19])
        if voltage_mv == VOLTAGE_UNAVAILABLE:
            print("System Voltage  : UNAVAILABLE (0xFFFF)")
        else:
            print(f"System Voltage  : {voltage_mv / 1000.0:.3f} V ({voltage_mv} mV)")
    else:
        print("System Voltage  : NOT PRESENT (version 1)")

    return True


# ============================================================
# Command handlers
# ============================================================

def command_set_interval(args):

    packet = build_set_interval(
        args.sequence,
        args.minutes
    )

    crc = read_uint16_be(
        packet[-2:]
    )

    print()
    print("========================================")
    print(" SET_INTERVAL Command")
    print("========================================")

    print(f"Sequence : {args.sequence}")
    print(f"Interval : {args.minutes} minutes")

    if args.minutes == 0:
        print("Mode     : Telemetry OFF")

    print()
    print(f"CRC      : 0x{crc:04X}")
    print(f"HEX      : {bytes_to_hex(packet)}")
    print(f"Base64   : {bytes_to_base64(packet)}")

    print()
    print(
        "Cloudloop MT payload:"
    )

    print(
        bytes_to_base64(packet)
    )


def command_decode_hex(args):

    try:
        clean_hex = (
            args.data
            .replace(" ", "")
            .replace(":", "")
            .replace("-", "")
        )

        packet = bytes.fromhex(clean_hex)

    except ValueError:
        print("ERROR: Invalid hexadecimal data")
        sys.exit(1)

    if not decode_packet(packet):
        sys.exit(1)


def command_decode_base64(args):

    try:
        packet = base64.b64decode(
            args.data,
            validate=True
        )

    except (ValueError, binascii.Error):
        print("ERROR: Invalid Base64 data")
        sys.exit(1)

    if not decode_packet(packet):
        sys.exit(1)


# ============================================================
# Argument parser
# ============================================================

def create_parser():

    parser = argparse.ArgumentParser(
        description=(
            "Vessel Tracker binary protocol utility"
        )
    )

    subparsers = parser.add_subparsers(
        dest="command",
        required=True
    )


    # --------------------------------------------------------
    # SET_INTERVAL
    # --------------------------------------------------------

    set_interval_parser = subparsers.add_parser(
        "set-interval",
        help="Generate a SET_INTERVAL MT command"
    )

    set_interval_parser.add_argument(
        "minutes",
        type=int,
        help="Reporting interval in minutes (0-1440)"
    )

    set_interval_parser.add_argument(
        "--sequence",
        "-s",
        type=int,
        default=1,
        help="Command sequence number (default: 1)"
    )

    set_interval_parser.set_defaults(
        func=command_set_interval
    )


    # --------------------------------------------------------
    # Decode HEX
    # --------------------------------------------------------

    hex_parser = subparsers.add_parser(
        "decode-hex",
        help="Decode a protocol packet from hexadecimal"
    )

    hex_parser.add_argument(
        "data",
        help="Hexadecimal packet"
    )

    hex_parser.set_defaults(
        func=command_decode_hex
    )


    # --------------------------------------------------------
    # Decode Base64
    # --------------------------------------------------------

    base64_parser = subparsers.add_parser(
        "decode-base64",
        help="Decode a protocol packet from Base64"
    )

    base64_parser.add_argument(
        "data",
        help="Base64 packet"
    )

    base64_parser.set_defaults(
        func=command_decode_base64
    )

    return parser


# ============================================================
# Main
# ============================================================

def main():

    parser = create_parser()

    args = parser.parse_args()

    try:
        args.func(args)

    except ValueError as error:
        print(f"ERROR: {error}")
        sys.exit(1)


if __name__ == "__main__":
    main()
