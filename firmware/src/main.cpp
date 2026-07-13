#include <Arduino.h>
#include "RobotConfig.h"
#include "Version.h"
#include "Constants.h"
#include "Types.h"
#include "Watchdog.h"

Watchdog watchdog;

void setup()
{
    Serial.begin(115200);
    delay(500);

    watchdog.begin();
    Serial.println("Watchdog started.");

    Serial.println("================================");
    Serial.println(Version::PROJECT_NAME);
    Serial.println(Version::COMPETITION_NAME);
    Serial.print("Software Version: ");
    Serial.println(Version::SOFTWARE_VERSION);
    Serial.print("Build Type: ");
    Serial.println(Version::BUILD_TYPE);
    Serial.println("System starting...");
    Serial.println("================================");
}

void loop(){

    watchdog.feed();

    if (watchdog.hasTimedOut())
    {
        Serial.println("ERROR: Watchdog timeout!");
    }
}