#include <Arduino.h>
#include "app/RobotApplication.h"

RobotApplication robotApplication;

void setup() {
    robotApplication.begin();
}

void loop() {
    robotApplication.update();
}
