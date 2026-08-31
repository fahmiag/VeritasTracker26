#include "SDCard.h"

#include "../../config/PinMap.h"


SDCard::SDCard()
    : spi_(VSPI),
      ready_(false),
      csPin_(PinMap::SD_CS)
{
}


bool SDCard::Begin()
{
    spi_.begin(
        PinMap::SD_SCK,
        PinMap::SD_MISO,
        PinMap::SD_MOSI,
        PinMap::SD_CS
    );


    if (!SD.begin(csPin_, spi_))
    {
        Serial.println("SD: initialization failed");

        ready_ = false;

        return false;
    }


    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE)
    {
        Serial.println("SD: no card detected");

        ready_ = false;

        return false;
    }


    ready_ = true;

    Serial.println("SD: initialized");

    return true;
}


bool SDCard::IsReady() const
{
    return ready_;
}


bool SDCard::Exists(const char* path)
{
    if (!ready_)
    {
        return false;
    }

    return SD.exists(path);
}


bool SDCard::CreateDirectory(const char* path)
{
    if (!ready_)
    {
        return false;
    }

    if (SD.exists(path))
    {
        return true;
    }

    return SD.mkdir(path);
}
File SDCard::OpenAppend(const char* path)
{
    if (!ready_)
        return File();

    return SD.open(path, FILE_APPEND);
}


bool SDCard::Write(
    File& file,
    const char* data)
{
    if (!file)
        return false;

    return file.print(data) > 0;
}


bool SDCard::WriteLine(
    File& file,
    const char* data)
{
    if (!file)
        return false;

    if (file.println(data) == 0)
        return false;

    return true;
}


bool SDCard::Flush(File& file)
{
    if (!file)
        return false;

    file.flush();

    return true;
}


bool SDCard::Close(File& file)
{
    if (!file)
        return false;

    file.close();

    return true;
}

//=======================
bool SDCard::AppendFile(
    const char* path,
    const char* data)
{
    if (!ready_)
    {
        return false;
    }


    File file = SD.open(
        path,
        FILE_APPEND
    );


    if (!file)
    {
        Serial.print("SD: failed to open ");

        Serial.println(path);

        return false;
    }


    size_t written =
        file.print(data);


    file.close();


    return written > 0;
}


bool SDCard::WriteFile(
    const char* path,
    const char* data)
{
    if (!ready_)
    {
        return false;
    }


    File file = SD.open(
        path,
        FILE_WRITE
    );


    if (!file)
    {
        Serial.print("SD: failed to open ");

        Serial.println(path);

        return false;
    }


    size_t written =
        file.print(data);


    file.close();


    return written > 0;
}


bool SDCard::ReadFile(
    const char* path)
{
    if (!ready_)
    {
        return false;
    }


    File file = SD.open(
        path,
        FILE_READ
    );


    if (!file)
    {
        return false;
    }


    Serial.print("SD: ");

    Serial.println(path);


    while (file.available())
    {
        Serial.write(
            file.read()
        );
    }


    file.close();

    return true;
}


uint64_t SDCard::FileSize(
    const char* path)
{
    if (!ready_)
    {
        return 0;
    }


    File file = SD.open(
        path,
        FILE_READ
    );


    if (!file)
    {
        return 0;
    }


    uint64_t size =
        file.size();


    file.close();


    return size;
}