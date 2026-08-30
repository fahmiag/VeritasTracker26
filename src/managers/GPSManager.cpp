
#include "GPSManager.h"

#include <Arduino.h>

// ------------------------------------------------------------
// Configuration
// ------------------------------------------------------------

// SCX-20 is considered stale if no valid position has been
// received within this time.
//
// 129029 is received as a Fast Packet, so use a few seconds
// of timeout rather than only 1 second.
static constexpr uint32_t SCX20_POSITION_TIMEOUT_MS = 3000;


// ------------------------------------------------------------
// Constructor
// ------------------------------------------------------------

GPSManager::GPSManager(
    SCX20CAN& scx20,
    UBloxGNSS& ublox
)
    : scx20_(scx20),
      ublox_(ublox)
{
}


// ------------------------------------------------------------
// Begin
// ------------------------------------------------------------

bool GPSManager::Begin()
{
    bool scx20Ok = scx20_.Begin();
    bool ubloxOk = ublox_.Begin();

    Serial.println("[GPS] Starting GPS sources...");

    Serial.print("[GPS] SCX-20: ");
    Serial.println(
        scx20Ok ? "OK" : "FAILED"
    );

    Serial.print("[GPS] u-blox: ");
    Serial.println(
        ubloxOk ? "OK" : "FAILED"
    );

    return scx20Ok || ubloxOk;
}


// ------------------------------------------------------------
// Update
// ------------------------------------------------------------

void GPSManager::Update()
{
    // --------------------------------------------------------
    // Update both GPS sources.
    //
    // We always update both sources even when SCX-20 is active.
    // This allows u-blox to remain ready as a fallback.
    // --------------------------------------------------------

    scx20_.Update();

    ublox_.Update();
}


// ------------------------------------------------------------
// Get Position
// ------------------------------------------------------------

bool GPSManager::GetPosition(
    PositionData& position
)
{
    PositionData scx20Position;

    // --------------------------------------------------------
    // Priority 1: SCX-20
    // --------------------------------------------------------

    // if (scx20_.GetPosition(scx20Position))
    // {
    //     uint32_t age =
    //         millis() - scx20Position.lastUpdate;

    //     if (age <= SCX20_POSITION_TIMEOUT_MS)
    //     {
    //         position = scx20Position;

    //         return true;
    //     }
    // }

    //temporary use uBlox as main gps source
    // --------------------------------------------------------
    // Priority 2: u-blox
    // --------------------------------------------------------

    PositionData ubloxPosition;

    if (ublox_.GetPosition(ubloxPosition))
    {
        position = ubloxPosition;

        return true;
    }


    // --------------------------------------------------------
    // No valid position available
    // --------------------------------------------------------

    return false;
}


// ------------------------------------------------------------
// Get Heading
// ------------------------------------------------------------

bool GPSManager::GetHeading(
    HeadingData& heading
)
{
    // --------------------------------------------------------
    // SCX-20 is currently the preferred heading source.
    // --------------------------------------------------------

    HeadingData scx20Heading;

    if (scx20_.GetHeading(scx20Heading))
    {
        uint32_t age =
            millis() - scx20Heading.lastUpdate;

        if (age <= SCX20_POSITION_TIMEOUT_MS)
        {
            heading = scx20Heading;

            return true;
        }

        
    }


 

    return false;
}
