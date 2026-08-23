# Vessel Tracking Telemetry Protocol

**Protocol Version:** 1  
**Message Type:** POSITION (`0x01`)  
**Packet Size:** 22 bytes  
**Byte Order:** Little-endian

## 1. Overview

This document defines the binary telemetry packet transmitted by the Vessel Tracking System through the RockBLOCK 9704 satellite modem.

The protocol is designed to provide a compact, deterministic representation of vessel position and navigation data.

The Version field identifies the protocol format. The Message Type identifies the kind of telemetry packet. The GPS Source field identifies which navigation source supplied the position.

## 2. Packet Format

A POSITION packet is exactly 22 bytes.

| Byte(s) | Size | Field | Type | Encoding |
|---------|-----:|-------|------|----------|
| 0 | 1 | Version | `uint8` | Protocol version |
| 1 | 1 | Type | `uint8` | `0x01` = POSITION |
| 2 | 1 | Flags | `uint8` | Validity flags |
| 3 | 1 | Source | `uint8` | Bits 0-1 = GPS source; bits 2-7 reserved |
| 4–5 | 2 | Sequence | `uint16` | Little-endian |
| 6–9 | 4 | Timestamp | `uint32` | Unix UTC seconds, little-endian |
| 10–13 | 4 | Latitude | `int32` | Degrees × 10,000,000 |
| 14–17 | 4 | Longitude | `int32` | Degrees × 10,000,000 |
| 18–19 | 2 | COG | `uint16` | Degrees × 100 |
| 20–21 | 2 | Heading | `uint16` | Degrees × 100 |

### Byte map

```text
Byte:    00  01  02  03  04  05  06  07  08  09  10  11  12  13  14  15  16  17  18  19  20  21
         ┌───┬───┬───┬───┬───────┬───────────────┬───────────────┬───────────────┬───────┬───────┐
Field:   │ V │ T │ F │ S │Sequence│   Timestamp   │   Latitude    │   Longitude   │  COG  │Heading│
         └───┴───┴───┴───┴───────┴───────────────┴───────────────┴───────────────┴───────┴───────┘
Size:     1   1   1   1     2            4                4                4          2       2
```

## 3. Field Definitions

### 3.1 Version

| Value | Meaning |
|---:|---|
| `0x01` | Protocol Version 1 |

The version identifies the packet format.

If an incompatible packet format is introduced in the future, the protocol version should be incremented.

### 3.2 Type

| Value | Meaning |
|---:|---|
| `0x01` | POSITION |

The current protocol defines only the POSITION message.

Additional message types can be added in future protocol versions.

### 3.3 Flags

The Flags field contains independent validity indicators.

| Bit | Mask | Meaning |
|---:|---:|---|
| 0 | `0x01` | GPS position valid |
| 1 | `0x02` | COG valid |
| 2 | `0x04` | Heading valid |
| 3 | `0x08` | Timestamp valid |
| 4–7 | — | Reserved |

A bit value of `1` means the corresponding field is valid.

Example:

```text
Flags = 0x0F
```

means:

```text
GPS valid
COG valid
Heading valid
Timestamp valid
```

Reserved bits must currently be transmitted as `0` and ignored by decoders.

### 3.4 GPS Source

The GPS Source field identifies the source of the position data.

| Value | Source |
|---:|---|
| `0x00` | NONE / unknown |
| `0x01` | UBLOX |
| `0x02` | SCX-20 |

This field is separate from the validity flags.

For example:

```text
GPS Source = 0x01
GPS Valid  = 1
```

means the valid position was obtained from the u-blox GNSS receiver.

Bits 0-1 : GPS Source
Bits 2-7 : Reserved bit, additional information (Future protocol expansion)
            Set to zero for now
             
Bit:  7 6 5 4 3 2 1 0
      └───────┬───────┘
       Reserved  GPS Source
        6 bits    2 bits

### 3.5 Sequence 

The sequence number is a 16-bit unsigned counter.

Range:

```text
0–65535
```

The sequence number increments for each telemetry packet.

Example:

```text
65534
65535
0
1
2
```

The counter wraps from `65535` to `0`.

The sequence number allows the receiving system to detect:

- Missing packets
- Duplicate packets
- Packet ordering

At one telemetry packet every 5 minutes, a 16-bit sequence counter covers approximately 227.6 days before wrapping.

### 3.6 Timestamp

The timestamp is a 32-bit unsigned Unix timestamp representing UTC time.

Encoding:

```text
Unix timestamp = seconds since 1970-01-01 00:00:00 UTC
```

The value is transmitted as a little-endian `uint32`.

The timestamp should represent the time associated with the position measurement.

### 3.7 Latitude

Latitude is encoded as a signed 32-bit integer.

```text
encoded_latitude = latitude_degrees × 10,000,000
```

Example:

```text
Latitude = 2.9687130°
Encoded  = 29687130
```

The decoder performs:

```text
latitude_degrees = encoded_latitude / 10,000,000
```

Negative values represent south latitude.

### 3.8 Longitude

Longitude is encoded as a signed 32-bit integer.

```text
encoded_longitude = longitude_degrees × 10,000,000
```

Example:

```text
Longitude = 101.8543210°
Encoded   = 1018543210
```

The decoder performs:

```text
longitude_degrees = encoded_longitude / 10,000,000
```

Negative values represent west longitude.

### 3.9 COG

Course Over Ground (COG) is encoded as an unsigned 16-bit integer.

```text
encoded_cog = COG_degrees × 100
```

Resolution:

```text
0.01°
```

Example:

```text
COG = 123.45°
Encoded = 12345
```

The decoder performs:

```text
COG_degrees = encoded_cog / 100
```

The normal compass range is:

```text
0° ≤ COG < 360°
```

### 3.10 Heading

Heading is encoded as an unsigned 16-bit integer.

```text
encoded_heading = heading_degrees × 100
```

Resolution:

```text
0.01°
```

Example:

```text
Heading = 271.35°
Encoded = 27135
```

The decoder performs:

```text
heading_degrees = encoded_heading / 100
```

The normal compass range is:

```text
0° ≤ Heading < 360°
```

## 4. Byte Order

All multi-byte integer fields use **little-endian** byte order.

For example:

```text
Sequence = 0x1234
```

is transmitted as:

```text
34 12
```

A 32-bit value:

```text
0x12345678
```

is transmitted as:

```text
78 56 34 12
```

Signed integers use two's-complement representation.

## 5. C++ Representation

The logical application packet is represented by `TelemetryPacket`.

The packet structure must **not** be transmitted directly using `sizeof(TelemetryPacket)`.

Instead, `TelemetryEncoder` explicitly serializes each field into the defined 22-byte format.

Conceptually:

```text
TelemetryPacket
      |
      v
TelemetryEncoder
      |
      v
22-byte binary buffer
      |
      v
RockBLOCK 9704
```

This prevents compiler padding, alignment, or native CPU byte order from becoming part of the protocol.

## 6. Example Packet

Example logical values:

```text
Version      = 1
Type         = POSITION
Flags        = 0x0F
GPS Source   = UBLOX
Sequence     = 100
Timestamp    = <Unix UTC timestamp>
Latitude     = 2.9687130°
Longitude    = 101.8543210°
COG          = 123.45°
Heading      = 271.35°
```

The exact transmitted HEX depends on the timestamp value.

For a sequence of `100`, the sequence bytes are:

```text
64 00
```

The decoder should reconstruct the original logical values from the 22-byte packet.

## 7. Decoder Requirements

A compatible decoder must:

1. Verify that the packet contains exactly 22 bytes for a POSITION packet.
2. Read Version from byte 0.
3. Read Type from byte 1.
4. Decode Flags from byte 2.
5. Decode GPS Source from byte 3.
6. Decode Sequence as little-endian `uint16`.
7. Decode Timestamp as little-endian `uint32`.
8. Decode Latitude as little-endian signed `int32`, then divide by `10,000,000`.
9. Decode Longitude as little-endian signed `int32`, then divide by `10,000,000`.
10. Decode COG as little-endian `uint16`, then divide by `100`.
11. Decode Heading as little-endian `uint16`, then divide by `100`.

Unknown GPS source values should not be interpreted as a known source.

Unknown flag bits should be ignored.

## 8. Protocol Compatibility

The protocol version is part of every packet.

A decoder should check the Version field before interpreting the remaining packet fields.

For Version 1:

```text
Packet size = 22 bytes
```

Changes that alter field positions, field sizes, encoding, or byte order should result in a new protocol version.

Changes that do not alter the binary packet format may be documented without changing the protocol version.

## 9. Reserved Space

The following fields currently contain reserved values:

```text
Flags bits 4–7
```

Reserved values are transmitted as zero in Version 1.

Future protocol versions may assign meanings to reserved bits.

## 10. Reference Implementation

The firmware implementation is located in:

```text
src/Satellite/TelemetryPacket.h
src/Satellite/TelemetryEncoder.h
src/Satellite/TelemetryEncoder.cpp
src/Satellite/TelemetryConfig.h
```

The protocol documentation is located at:

```text
docs/telemetry_protocol.md
```
