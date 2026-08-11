#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

class SDCard
{
public:

    SDCard();

    bool Begin();

    bool IsReady() const;

    bool Exists(const char* path);

    bool CreateDirectory(const char* path);

    bool AppendFile(
        const char* path,
        const char* data
    );

    bool WriteFile(
        const char* path,
        const char* data
    );

    bool ReadFile(
        const char* path
    );

    uint64_t FileSize(
        const char* path
    );

private:

    SPIClass spi_;

    bool ready_;

    uint8_t csPin_;
};