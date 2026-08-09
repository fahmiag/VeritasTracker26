#include "ButtonManager.h"

bool ButtonManager::Begin()
{
    return buttons_.Begin();
}

void ButtonManager::Update()
{
    lastButton_ = buttons_.Update();
}

Button ButtonManager::GetLastButton()
{
    return lastButton_;
}

bool ButtonManager::WasPressed(Button button)
{
    return lastButton_ == button;
}