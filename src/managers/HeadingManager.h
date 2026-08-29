#pragma once
#include <Arduino.h>

#include "../drivers/SCX20CAN.h"
#include "../data/HeadingData.h"

class HeadingManager
{
public:
    HeadingManager();

    bool Begin();

    void Update();
  
    bool GetHeading(HeadingData& heading);
    
    bool IsValid() const;

private:


    HeadingData headingData_;

    SCX20CAN scx20_;

};