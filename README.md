This code running on VS Code + Platform IO

=============
Python Tools
=============

protocol_tool.py
-----------------
Decode string
> python tools/protocol_tool.py decode-base64 AYAABwIAHqDz
> python tools/protocol_tool.py decode-hex 0180000702001EA0F3

Create Cloudloop MT payload:
calculate CRC, HEX and Base64
Set 30 minutes interval with sequence 7
> python tools/protocol_tool.py set-interval 30 --sequence 7 

    Sequence : 7
    Interval : 30 minutes

    CRC      : 0xA0F3
    HEX      : 0180000702001EA0F3
    Base64   : AYAABwIAHqDz

    Cloudloop MT payload:
    AYAABwIAHqDz

> python tools/protocol_tool.py set-interval 5 --sequence 67  

Sequence : 67
Interval : 5 minutes

CRC      : 0xA7C4
HEX      : 01800043020005A7C4
Base64   : AYAAQwIABafE

Cloudloop MT payload:
AYAAQwIABafE


protocol_telemetry_V1.py
----------------------
Can decode only, not encode
Decode telemetry HEX 
> python tools/decode_telemetry.py

protocol_telemetry.py
----------------------
Old protocol, for the first test binary string
Decode telemetry HEX 
> python tools/decode_telemetry.py


=============
V0.1.0-dev
=============
* Basic Tracking
* 5-minute interval (Fix)
* Position source : GNSS, uBlox (RockBLOCKPlus 9704)
* Heading source : Furuno SCX-20
* No Over-The-Air (OTA) message
* No Postion GNSS from SCX-20
* Display value not correct

TODO:
- Set GPS source to uBlox
- Set retry time for unsend message
- dont increase sequence number when message not sent.


=============
V0.2.0-dev
=============
* OTA message to change interval
