#include "Watchdog.h"
#include "Constants.h"

void Watchdog::begin()
{
    lastFeedTime = millis();
}

void Watchdog::feed()
{
    lastFeedTime = millis();
}

bool Watchdog::hasTimedOut()
{
    return (millis() - lastFeedTime) > WATCHDOG_TIMEOUT_MS;
}

unsigned long Watchdog::getLastFeedTime() const
{
    return lastFeedTime;
}