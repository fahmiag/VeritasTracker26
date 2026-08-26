
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

    if (
        CAN_MSGAVAIL ==
        CAN.checkReceive()
    )
    {
        CAN.readMsgBuf(
            &len,
            buf
        );

        uint32_t canId =
            CAN.getCanId();

        Serial.print(
            "CAN ID: 0x"
        );

        Serial.print(
            canId,
            HEX
        );

        Serial.print(
            " DLC: "
        );

        Serial.print(
            len
        );

        Serial.print(
            " DATA: "
        );

        for (
            uint8_t i = 0;
            i < len;
            i++
        )
        {
            if (buf[i] < 0x10)
            {
                Serial.print("0");
            }

            Serial.print(
                buf[i],
                HEX
            );

            Serial.print(" ");
        }

        Serial.println();
    }
}