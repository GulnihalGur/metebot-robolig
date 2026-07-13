#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <Arduino.h>

class Watchdog
{
public:
    void begin();
    void feed();
    bool hasTimedOut();
    unsigned long getLastFeedTime() const;

private:
    unsigned long lastFeedTime;
};

#endif