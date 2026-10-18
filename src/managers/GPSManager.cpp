
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
    Serial.println("[GPS] Starting GPS sources...");

    scx20Initialized_ = scx20_.Begin();
    lastScx20InitAttempt_ = millis();

    const bool ubloxStarted = ublox_.Begin();

    Serial.print("[GPS] SCX20 CAN: ");
    Serial.println(
        scx20Initialized_
            ? "OK"
            : "FAILED; retry in 60 seconds"
    );

    Serial.print("[GPS] u-blox UART: ");
    Serial.println(ubloxStarted ? "STARTED" : "FAILED");

    return scx20Initialized_ || ubloxStarted;
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

        // Keep processing u-blox even while CAN is unavailable.
    ublox_.Update();

    if (!scx20Initialized_)
    {
        if (uint32_t(millis() - lastScx20InitAttempt_) >=
            INIT_RETRY_MS)
        {
            Serial.println("[GPS] Retrying SCX20 CAN initialization");

            scx20Initialized_ = scx20_.Begin();
            lastScx20InitAttempt_ = millis();

            Serial.println(
                scx20Initialized_
                    ? "[GPS] SCX20 CAN initialized"
                    : "[GPS] SCX20 CAN failed; retry in 60 seconds"
            );
        }

        return;
    }

    scx20_.Update();


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
