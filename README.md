# VeritasTracker26

Embedded satellite tracking system built on the ESP32, combining satellite
communications with GNSS-based heading data over CAN bus.

## Features

- ESP32 microcontroller firmware (PlatformIO)
- Satellite telemetry via RockBLOCK Plus 9704 modem
- GNSS from uBlox inside RockBLOCK Plus 9704 modem
- GNSS heading/compass data from a Furuno SCX-20 over CAN bus (NMEA2000)
- Telemetry encoding pipeline for uplink messages

## Hardware Requirements

- ESP32 development board: NodeMCU with ESP32
- RockBLOCK Plus 9704 satellite modem
- Furuno SCX-20 GNSS compass
- CAN transceiver module (for NMEA2000 bus)


## Software Requirements

- [PlatformIO](https://platformio.org/) (CLI or VS Code extension)
- VS Code (recommended)
- <!-- list any additional libraries and pinned versions from platformio.ini -->

## Project Structure

```
VeritasTracker26/
├── docs/telemetry_protocol.md
├── src/            # Application source (App::Run, main loop)
│   ├── app          
│   ├── config
│   ├── data        
│   ├── drivers     # Communicate with hardware
│   ├── managers
│   ├── protocol    # Satellite communication protocol
│   ├── services
│   ├── utils
│   └── ...
├── lib/            # Project-specific libraries
│   └── MCP_CAN/   # Satellite message encoding
├── platformio.ini   # Build configuration and dependencies
└── README.md
```

<!-- Adjust the tree above to match your actual layout -->

## Configuration

- Board and environment settings live in `platformio.ini`.
- <!-- Note any secrets/API keys and how they're supplied (e.g. a gitignored
  include/secrets.h, or environment variables) -->




Python Tools
=============

protocol_tool.py
-----------------
### Decode string


```
python tools/protocol_tool.py decode-base64 AYAABwIAHqDz
    
python tools/protocol_tool.py decode-hex 0180000702001EA0F3
```

### Create Cloudloop MT payload:

calculate CRC, HEX and Base64

* Set 30 minutes interval with sequence 7


```
python tools/protocol_tool.py set-interval 30 --sequence 7 
```
Output:

```
    Sequence : 7
    Interval : 30 minutes

    CRC      : 0xA0F3
    HEX      : 0180000702001EA0F3
    Base64   : AYAABwIAHqDz

    Cloudloop MT payload:
    AYAABwIAHqDz
```


* Set 5 minutes interval with sequence 67
```
python tools/protocol_tool.py set-interval 5 --sequence 67  
```
Output:
```
    Sequence : 67
    Interval : 5 minutes

    CRC      : 0xA7C4
    HEX      : 01800043020005A7C4
    Base64   : AYAAQwIABafE

    Cloudloop MT payload:
    AYAAQwIABafE
```

protocol_telemetry_V1.py
----------------------
Can decode only, not encode
Decode telemetry HEX 

```
python tools/decode_telemetry.py
```
protocol_telemetry.py
----------------------
Old protocol, for the first test binary string
Decode telemetry HEX 
```
python tools/decode_telemetry.py
```

Code Version
=============

V0.1.0-dev
-----------------
* Basic Tracking
* 5-minute interval (Fix)
* Position source : GNSS, uBlox (RockBLOCKPlus 9704)
* Heading source : Furuno SCX-20
* No Over-The-Air (OTA) message
* No Postion GNSS from SCX-20
* Display value not correct


V0.2.0-dev
--------------
* OTA message to change interval


TODO
=============
- Set GPS source to uBlox
- Set retry time for unsend message
- dont increase sequence number when message not sent.
- Add Panic Button alert and Voltage low alert
- Add Panic button status and Voltage value to position payload

## Known Issues

Tracking open bugs found during code review:

- [ ] SCX-20 CAN string too much, high trafic. Try to reduce it.
- [ ] MicroSD Card not working

