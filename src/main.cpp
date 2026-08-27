
// #include "app/App.h"

// Adafruit_SSD1306 display(128, 64, &Wire, -1);

// App app;

// void setup()
// {
//     app.Begin();

// }

// void loop()
// {
//     app.Run();
// }

#include <Arduino.h>
#include <SPI.h>
#include "mcp_can.h"

const int SPI_CS_PIN = 5;

MCP_CAN CAN(SPI_CS_PIN);
uint32_t ExtractPGN(uint32_t canId) ;

void setup()
{
    Serial.begin(115200);

    SPI.begin();

    delay(1000);

    while (
        CAN_OK != CAN.begin(CAN_500KBPS)
    )
    {
        Serial.println(
            "CAN BUS Shield init fail"
        );

        Serial.println(
            "Init CAN BUS Shield again"
        );

        delay(100);
    }

    Serial.println(
        "CAN BUS Shield init ok!"
    );
}

void loop()
{
    unsigned char len = 0;
    unsigned char buf[8];

    if (CAN_MSGAVAIL == CAN.checkReceive())
    {
        CAN.readMsgBuf(
            &len,
            buf
        );

        uint32_t canId =
            CAN.getCanId();

        // Serial.print(
        //     "CAN ID: 0x"
        // );

        // Serial.print(
        //     canId,
        //     HEX
        // );

        // Serial.print(
        //     " DLC: "
        // );

        // Serial.print(
        //     len
        // );

        // Serial.print(
        //     " DATA: "
        // );

        // for (
        //     uint8_t i = 0;
        //     i < len;
        //     i++
        // )
        // {
        //     if (buf[i] < 0x10)
        //     {
        //         Serial.print("0");
        //     }

        //     Serial.print(
        //         buf[i],
        //         HEX
        //     );

        //     Serial.print(" ");
        // }
        //========================================
        uint32_t pgn = ExtractPGN(canId);

        Serial.print("[SCX20] ID: 0x");
        Serial.print(canId, HEX);

        Serial.print("  PGN: ");
        Serial.print(pgn);

        Serial.print("  DATA: ");

        for (uint8_t i = 0; i < len; i++)
        {
            if (buf[i] < 0x10)
                Serial.print("0");

            Serial.print(buf[i], HEX);
            Serial.print(" ");
        }

        Serial.println();
    }
}

uint32_t ExtractPGN(uint32_t canId) {
  uint8_t dataPage     = (canId >> 24) & 0x01;
  uint8_t pduFormat    = (canId >> 16) & 0xFF;
  uint8_t pduSpecific  = (canId >> 8) & 0xFF;

  uint32_t pgn;

  if (pduFormat < 240) {
    // PDU1 format (destination-specific PGN)
    pgn = ((uint32_t)dataPage << 16) | ((uint32_t)pduFormat << 8);
  } else {
    // PDU2 format (broadcast PGN)
    pgn = ((uint32_t)dataPage << 16) | ((uint32_t)pduFormat << 8) | pduSpecific;
  }

  return pgn;
}
