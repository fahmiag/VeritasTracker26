#include "HeadingManager.h"


bool HeadingManager::Begin()
{
    return scx20_.Begin();
}


void HeadingManager::Update()
{
    scx20_.Update();

    scx20_.GetHeading(heading_);
}


bool HeadingManager::GetHeading(
    HeadingData& heading)
{
    heading = heading_;

    return heading_.valid;
}