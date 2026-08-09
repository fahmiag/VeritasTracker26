
#include "ButtonManager.h"


bool ButtonManager::Begin()
{
    return buttons_.Begin();
}


void ButtonManager::Update()
{
    lastEvent_ = buttons_.Update();
}


ButtonEvent ButtonManager::GetEvent()
{
    return lastEvent_;
}


bool ButtonManager::WasEvent(ButtonEvent event)
{
    return lastEvent_ == event;
}

