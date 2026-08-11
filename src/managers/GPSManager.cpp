#include "GPSManager.h"


bool GPSManager::Begin()
{
    return ublox_.Begin();
}


void GPSManager::Update()
{
    ublox_.Update();

    ublox_.GetPosition(ubloxPosition_);
}


bool GPSManager::GetPosition(PositionData& position)
{
    position = ubloxPosition_;

    return ubloxPosition_.valid;
}