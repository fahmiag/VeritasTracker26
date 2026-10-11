# Vessel Tracking Telemetry Protocol

**Protocol version:** 1 (unchanged)
**Byte order:** Big-endian (network byte order)
**Application CRC:** CRC-16/XMODEM (CCITT polynomial): poly `0x1021`, init
`0x0000`, refin=false, refout=false, xorout=`0x0000`.
The check value for ASCII `123456789` is `0x31C3`.

This document describes the firmware at commit
`219b67cde1fccfc700437f6bb0a5ca1827722ccd`. Wire definitions come from
`src/protocol/TelemetryConfig.h`, `TelemetryEncoder.cpp`, `ProtocolConfig.h`
and `ProtocolEncoder.cpp`; event behavior comes from `AlarmManager.cpp` and
`SatelliteManager.cpp`.

## 1. Common packet format

| Packet byte(s) | Size | Field | Encoding |
|---|---:|---|---|
| 0 | 1 | Version | `uint8`, currently `0x01` |
| 1 | 1 | Type | `uint8` |
| 2–3 | 2 | Sequence | `uint16` BE |
| 4 | 1 | Payload length | `uint8`, excludes header and CRC |
| 5 through 4+N | N | Payload | Message-specific |
| 5+N through 6+N | 2 | CRC | `uint16` BE |

Total length is `5 + N + 2`. CRC covers the header and payload, excluding the
trailing CRC. A packet must have the declared length, the correct type-specific
payload length, the supported version and a matching CRC.

## 2. Message types

| Type | Value | Direction | Payload bytes | Total bytes | Meaning |
|---|---:|---|---:|---:|---|
| POSITION | `0x01` | Device → server (MO) | 17 | 24 | Periodic navigation report |
| ACK | `0x02` | Device → server (MO) | 3 | 10 | Command result |
| EMERGENCY | `0x03` | Device → server (MO) | 17 | 24 | Emergency input activation |
| TAMPER | `0x04` | Device → server (MO) | 17 | 24 | Armed enclosure opening |
| POWER_LOSS | `0x05` | Device → server (MO) | 17 | 24 | PSU-to-battery transition |
| SET_INTERVAL | `0x80` | Server → device (MT) | 2 | 9 | Set periodic reporting interval |

The message type identifies a dedicated alert. Emergency and tamper status
bits may also appear in ordinary POSITION reports; those bits do not change
the message type. No separate power-loss status bit exists: bit 7 identifies
PSU power, while type `0x05` identifies the transition event.

## 3. Shared navigation payload

POSITION, EMERGENCY, TAMPER and POWER_LOSS use exactly the same payload and CRC
framing. No additional event ID or uptime field is appended.

| Payload byte(s) | Packet byte(s) | Size | Field | Encoding |
|---|---|---:|---|---|
| 0 | 5 | 1 | Status | Bit field below |
| 1–4 | 6–9 | 4 | Timestamp | Unix UTC seconds, `uint32` BE |
| 5–8 | 10–13 | 4 | Latitude | Signed `int32` BE, degrees × 10,000,000 |
| 9–12 | 14–17 | 4 | Longitude | Signed `int32` BE, degrees × 10,000,000 |
| 13–14 | 18–19 | 2 | Course over ground (COG) | `uint16` BE, degrees × 100 |
| 15–16 | 20–21 | 2 | Heading | `uint16` BE, degrees × 100 |

The CRC occupies packet bytes 22–23. Divide coordinates by `10,000,000` and
angles by `100` when decoding. Firmware floating-point-to-integer casts truncate.

### Status flags

| Bit | Mask | Meaning when set | Meaning when clear |
|---:|---:|---|---|
| 0 | `0x01` | Position valid | Coordinates invalid |
| 1 | `0x02` | COG valid | COG invalid |
| 2 | `0x04` | Heading valid | Heading invalid |
| 3 | `0x08` | Timestamp valid | Timestamp invalid |
| 4 | `0x10` | Position source is SCX-20 | Source selection is u-blox |
| 5 | `0x20` | Emergency alarm latched | Emergency latch clear |
| 6 | `0x40` | Tamper alarm latched | Tamper latch clear |
| 7 | `0x80` | Confirmed selected source is PSU | Source is battery, USB, unknown or not initialized |

All eight bits are assigned; there are no reserved bits. The encoder sets or
clears bit 4 from `TelemetryPacket::positionSource`. Bit 4 alone does not imply
a valid fix. Bit 7 being clear does **not** prove battery operation.

Servers must honor the validity bits. Zero coordinates with POSITION_VALID=0
are not a location, and timestamp zero with TIME_VALID=0 is not a valid event
time. Raw invalid fields can be displayed for diagnostics but must not be used
for mapping, navigation or timestamp interpretation. Heading validity is
independent of position validity.

Version 1 has been retained despite the added types and status bits. Older
server decoders that reject these types or require bits 5–7 to be zero need
updating.

### Alert behavior

- Emergency and enclosure inputs are debounced for 50 ms. An active input at
  startup also raises an event once debounced.
- Emergency and tamper latch until explicitly acknowledged with the input
  inactive and its alert no longer outstanding. Releasing the input or a
  successful modem completion does not itself clear the latch.
- Tamper starts armed. Disarming permits maintenance; rearming an already-open
  enclosure raises an event. Disarming does not erase a pending event or latch.
- POWER_LOSS is not latched. `UpdatePowerSource()` establishes a startup
  baseline without an alert, then requires a changed source to remain stable
  for 500 ms. Only PSU → battery generates an event. Battery → PSU and startup
  on battery do not generate one.
- Alert navigation/status are captured when the event is queued, not refreshed
  for retries. Timestamp is the captured GNSS time, not a separately maintained
  button-event clock. Alerts can be encoded and queued without valid GPS.
- The next eligible event is selected in order: emergency, tamper, power loss.
  An existing modem transmission is not preempted. Retry backoff can make a
  lower-priority event eligible first.
- One event per kind is retained while pending/in flight. Repeated activations
  of that kind are coalesced; this is not a complete event-history FIFO.
- A queue rejection or failed modem completion starts a 30-second retry delay.
  Retries after an accepted queue retain the same sequence and packet bytes.
  Modem completion is not an application acknowledgement from a human/server.
- Latches, pending events and uplink sequences are stored in RAM and reset on
  reboot. A modem operation without a terminal completion can prevent further
  sends; application retries require that operation to finish or be recovered.

**Current integration limitation:** at the commit above, `App::Run()` does not
call `AlarmManager::UpdatePowerSource()`. The wire type and encoder support are
present, but automatic POWER_LOSS generation and PSU flag tracking are not
connected to `VoltageMonitor`; the alarm manager's PSU flag remains clear.
This document specifies the available format without claiming that this
missing application integration is active.

## 4. SET_INTERVAL

Payload length: **2 bytes**. Total packet: **9 bytes**.

| Payload byte(s) | Size | Field |
|---|---:|---|
| 0–1 | 2 | Reporting interval in minutes, `uint16` BE |

- `0`: disable periodic POSITION reports. This does not disable alarm scheduling.
- `1–1440`: periodic reporting interval in minutes.
- Values above `1440` are rejected by the configuration manager.

The accepted value is stored in NVS. With `DEBUG_SATELLITE_TX=1`, configuration
startup currently overrides the stored value with its default.

## 5. ACK

Payload length: **3 bytes**. Total packet: **10 bytes**.

| Payload byte | Size | Field |
|---:|---:|---|
| 0 | 1 | Acknowledged message type, e.g. `0x80` for SET_INTERVAL |
| 1 | 1 | Result |
| 2 | 1 | Error code |

| Result | Value |
|---|---:|
| SUCCESS | `0x00` |
| FAILED | `0x01` |

The wire error definitions are `Protocol::AckError`, used by the actual ACK
encoder and command handler:

| Error | Value |
|---|---:|
| NONE | `0x00` |
| INVALID_COMMAND | `0x01` |
| INVALID_VALUE | `0x02` |
| UNSUPPORTED_VERSION | `0x03` |
| INVALID_CRC | `0x04` |
| INTERNAL_ERROR | `0x05` |

`Telemetry::AckError` contains a different, currently unused name mapping; it
is not the authority for transmitted ACK error meanings.

ACK sequence echoes the command sequence and does not consume an uplink
sequence. The current SET_INTERVAL handler emits SUCCESS/NONE or
FAILED/INVALID_VALUE. Decoder-rejected packets (such as bad CRC/version) are
logged and dropped, not automatically answered with an error ACK. If the modem
is busy, the current handler can fail to queue the ACK and does not retain it
for retry.

## 6. Sequence numbers and duplicates

Device uplinks and server downlinks have independent 16-bit sequence spaces.
New navigation messages share the device counter, which wraps at 65535 and
restarts at zero on reboot. Position attempts currently increment the counter
before encoding/queue success, so sequence gaps can occur. Alerts increment it
when their first transmission is accepted; subsequent retries reuse it. ACKs
echo the downlink command sequence instead.

The current firmware does **not** implement duplicate-command detection.
Repeated SET_INTERVAL commands are processed again. Sequence numbers alone are
not globally unique across wraparound or reboot. Server alert deduplication
must account for retransmissions and sequence reuse; the wire format has no
boot/session identifier.

## 7. Python tool and transport

`tools/protocol_tool.py` uses only the Python standard library. Existing CLI
commands remain available:

```bash
python tools/protocol_tool.py set-interval 5 --sequence 7
python tools/protocol_tool.py set-interval 0 --sequence 8
python tools/protocol_tool.py decode-hex "<complete packet hex>"
python tools/protocol_tool.py decode-base64 "<complete packet base64>"
```

`set-interval` prints HEX and Base64 for the complete application packet,
including its header and CRC. Use that Base64 value as the Cloudloop MT payload.
Base64 is a transport representation, not an additional application field.
The tool does not transmit messages itself. Its Python `build_ack()` helper
builds test ACK packets; ACK is an MO response, not a supported latch-clear MT
command.

The decoder supports all six types, prints all status bits and labels invalid
navigation fields. It returns a nonzero CLI exit status for invalid HEX/Base64,
version, packet/payload length, CRC, unsupported type, invalid interval or
undefined ACK result/error values. It does not independently verify GPS
freshness or the truth of validity flags supplied by the firmware.
