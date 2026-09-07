# Vessel Tracking Telemetry Protocol

**Protocol Version:** 1  
**Byte Order:** Big-endian (network byte order)  
**Application CRC:** CRC-16/CCITT, poly `0x1021`, init `0x0000`, refin=false, refout=false, xorout=`0x0000`

## 1. Common Packet Format

All application messages use the same envelope:

| Byte(s) | Size | Field | Encoding |
|---|---:|---|---|
| 0 | 1 | Version | `uint8` |
| 1 | 1 | Type | `uint8` |
| 2–3 | 2 | Sequence | `uint16`, big-endian |
| 4 | 1 | Payload Length | `uint8` |
| 5… | N | Payload | Message-specific |
| 5+N… | 2 | CRC | `uint16`, big-endian |

CRC is calculated over bytes 0 through `4 + Payload Length`, excluding the CRC itself.

## 2. Message Types

| Type | Value | Direction | Meaning |
|---|---:|---|---|
| POSITION | `0x01` | Device → Server | Vessel position/navigation |
| ACK | `0x02` | Device → Server | Application acknowledgement |
| SET_INTERVAL | `0x80` | Server → Device | Set reporting interval |

## 3. POSITION

Payload length: **17 bytes**. Total packet: **24 bytes**.

| Payload Byte | Size | Field | Encoding |
|---:|---:|---|---|
| 0 | 1 | Position Status | bit fields |
| 1–4 | 4 | Timestamp | Unix UTC seconds, `uint32` BE |
| 5–8 | 4 | Latitude | `int32`, degrees × 10,000,000 |
| 9–12 | 4 | Longitude | `int32`, degrees × 10,000,000 |
| 13–14 | 2 | COG | `uint16`, degrees × 100 |
| 15–16 | 2 | Heading | `uint16`, degrees × 100 |

### Position Status 
Temporary, Need to be finalized later.

| Bit | Mask | Meaning |
|---:|---:|---|
| 0 | `0x01` | Position valid |
| 1 | `0x02` | COG valid |
| 2 | `0x04` | Heading valid |
| 3 | `0x08` | Timestamp valid |
| 4 | `0x10` | Source: `1` = SCX-20, `0` = u-blox |
| 5–7 | — | Reserved; transmit as 0 |

### Position source

The source is encoded in bit 4 of Position Status.

- `0` = u-blox
- `1` = SCX-20

## 4. SET_INTERVAL

Payload length: **2 bytes**. Total packet: **9 bytes**.

| Payload Byte | Size | Field |
|---:|---:|---|
| 0–1 | 2 | Reporting interval in minutes, `uint16` BE |

Values:

- `0` = telemetry OFF
- `1–1440` = reporting interval in minutes

Values above 1440 are invalid.

## 5. ACK

Payload length: **3 bytes**. Total packet: **10 bytes**.

| Payload Byte | Size | Field |
|---:|---:|---|
| 0 | 1 | Acknowledged Message Type |
| 1 | 1 | Result |
| 2 | 1 | Error Code |

Result:

- `0x00` = SUCCESS
- `0x01` = FAILED

Error codes:

- `0x00` = NONE
- `0x01` = INVALID_LENGTH
- `0x02` = INVALID_VALUE
- `0x03` = UNSUPPORTED_TYPE
- `0x04` = CRC_ERROR
- `0x05` = VERSION_ERROR

For an ACK, the Sequence field echoes the sequence number of the command being acknowledged. The ACK does not consume a new uplink sequence number.

## 6. Sequence Numbers

Device → Server and Server → Device use independent sequence spaces.

The device uplink sequence increments for each new uplink message and wraps from `65535` to `0`.

A downlink command has its own sequence number. If the same command sequence is received again, the device can recognize it as a duplicate and resend the corresponding ACK without reapplying the command.
