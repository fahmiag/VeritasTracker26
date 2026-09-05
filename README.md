This code running on VS Code + Platform IO

=============
Python Tools
=============
Decode telemetry HEX 
> python tools/decode_telemetry_V1.py

> python tools/protocol_tool.py decode-base64 AYAABwIAHqDz
> python tools/protocol_tool.py decode-hex 0180000702001EA0F3

Cloudloop MT payload:
> python tools/protocol_tool.py set-interval 30 --sequence 7 
Set 30 minutes interval with sequence 7
calculate CRC, HEX and Base64

-------------
V0.1.0-dev
-------------
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


-------------
V0.2.0-dev
-------------
* OTA message to change interval
