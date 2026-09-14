#include "SDCard.h"

#include "../../config/PinMap.h"

// =====================================================
// Constructor
// ===================================================== 
SDCard::SDCard()
    : 
      ready_(false),
      csPin_(PinMap::SD_CS)
{
}

// =====================================================
// Begin
// =====================================================
bool SDCard::Begin()
{
    
    // -------------------------------------------------
    // SD Card CS
    //
    // SPI bus itself is initialized by App::Begin().
    // -------------------------------------------------
    pinMode( csPin_, OUTPUT);
    digitalWrite(csPin_, HIGH);


    // -------------------------------------------------
    // Initialize SD card using shared global SPI bus
    // -------------------------------------------------
    if (!SD.begin(csPin_, SPI))
    {
        Serial.println("SD: initialization failed");
        
        return false;
    }

    // -------------------------------------------------
    // Check card
    // -------------------------------------------------
    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE)
    {
        Serial.println("SD: no card detected");
       
        return false;
    }


    ready_ = true;

    Serial.println("SD: initialized");

    return true;
}

// =====================================================
// Ready
// =====================================================
bool SDCard::IsReady() const
{
    return ready_;
}

// =====================================================
// Exists
// =====================================================
bool SDCard::Exists(const char* path)
{
    if (!ready_)
    {
        return false;
    }

    return SD.exists(path);
}

// =====================================================
// Create Directory
// =====================================================
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

// =====================================================
// Open Append
// =====================================================
File SDCard::OpenAppend(const char* path)
{
    if (!ready_)
        return File();

    return SD.open(path, FILE_APPEND);
}

// =====================================================
// Write 
// =====================================================
bool SDCard::Write(
    File& file,
    const char* data)
{
    if (!file)
        return false;

    return file.print(data) > 0;
}

// =====================================================
// Write Line
// =====================================================
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

// =====================================================
// Flush
// =====================================================
bool SDCard::Flush(File& file)
{
    if (!file)
        return false;

    file.flush();

    return true;
}

// =====================================================
// Close
// =====================================================
bool SDCard::Close(File& file)
{
    if (!file)
        return false;

    file.close();

    return true;
}

// =====================================================
// Append File
// =====================================================
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