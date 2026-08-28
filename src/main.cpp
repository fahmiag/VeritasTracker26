
#include "app/App.h"

Adafruit_SSD1306 display(128, 64, &Wire, -1);

App app;

void setup()
{
    app.Begin();

}

void loop()
{
    app.Run();
}

