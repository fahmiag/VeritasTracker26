#include "GPSManager.h"


bool GPSManager::Begin()
{
    return ublox_.Begin();
}


void GPSManager::Update()
{
    ublox_.Update();

    ublox_.GetPosition(position_);
}


bool GPSManager::GetPosition(PositionData& position)
{
    position = position_;

    return position_.valid;
}