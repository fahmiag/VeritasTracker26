
#include "app/App.h"
#include <Arduino.h>  //Remove this after test

Adafruit_SSD1306 display(128, 64, &Wire, -1);

App app;

void setup()
{
    app.Begin();


}

void loop()
{
    app.Run();    //Actual Working Code
    //app.Update();   //Test Code
}
