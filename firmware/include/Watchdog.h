#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <Arduino.h>

class Watchdog
{
public:
    // Yazilimsal ve ESP32 Task Watchdog'u baslatir.
    bool begin();

    // Dongu basariyla tamamlandiginda iki watchdog'u besler.
    void feed();

    // Yazilimsal dongu gecikmesini kontrol eder.
    bool hasTimedOut() const;

    // Son basarili dongu zamanini dondurur.
    unsigned long getLastFeedTime() const;

    // ESP32 Task Watchdog baslatildi mi?
    bool hardwareReady() const;

private:
    unsigned long lastFeedTime = 0;

    bool softwareReady = false;
    bool taskWatchdogReady = false;

    // ESP32 Task Watchdog'u baslatir.
    bool beginTaskWatchdog();
};

#endif