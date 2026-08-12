#include "HeadingManager.h"


HeadingManager::HeadingManager()
{
    headingData_.valid = false;
    headingData_.source = HeadingSource::NONE;
    headingData_.heading = 0.0;
    headingData_.rateOfTurn = 0.0;
    headingData_.lastUpdate = 0;
}

bool HeadingManager::Begin()
{
    headingData_.valid = false;
    headingData_.source = HeadingSource::NONE;
    headingData_.heading = 0.0; 
    headingData_.rateOfTurn = 0.0; 
    headingData_.lastUpdate = 0;

    Serial.println("HeadingManager: initialized");

    return true;
}


void HeadingManager::Update()
{
    /*
     * SCX-20 CAN decoding will be added later.
     *
     * For now, there is no heading source.
     */


}




bool HeadingManager::GetHeading( HeadingData& heading)
{
    heading = headingData_; 
    return headingData_.valid;
}


bool HeadingManager::IsValid() const
{
    return headingData_.valid;
}