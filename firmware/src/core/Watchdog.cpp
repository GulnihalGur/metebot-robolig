#include "Watchdog.h"

#include <esp_err.h>
#include <esp_task_wdt.h>
#include <esp_arduino_version.h>

#include "Constants.h"

bool Watchdog::begin()
{
    lastFeedTime = millis();
    softwareReady = true;

    taskWatchdogReady = beginTaskWatchdog();

    return taskWatchdogReady;
}

void Watchdog::feed()
{
    // Yazilimsal watchdog zamanini yenile.
    lastFeedTime = millis();

    // Mevcut loop task'ini yenile.
    if (taskWatchdogReady)
    {
        const esp_err_t result =
            esp_task_wdt_reset();

        if (result != ESP_OK)
        {
            taskWatchdogReady = false;
        }
    }
}

bool Watchdog::hasTimedOut() const
{
    if (!softwareReady)
    {
        return false;
    }

    return (millis() - lastFeedTime) >
           SOFTWARE_WATCHDOG_TIMEOUT_MS;
}

unsigned long Watchdog::getLastFeedTime() const
{
    return lastFeedTime;
}

bool Watchdog::hardwareReady() const
{
    return taskWatchdogReady;
}

bool Watchdog::beginTaskWatchdog()
{
#if ESP_ARDUINO_VERSION_MAJOR >= 3

    // Arduino-ESP32 3.x / ESP-IDF 5.x API'si.
    esp_task_wdt_config_t config = {};

    config.timeout_ms =
        HARDWARE_WATCHDOG_TIMEOUT_MS;

    // Yalnizca ekledigimiz loop task'i izlenir.
    config.idle_core_mask = 0;

    // Timeout durumunda panic ve reset olustur.
    config.trigger_panic = true;

    esp_err_t result =
        esp_task_wdt_init(&config);

    // Daha once baslatildiysa ayarlari yenile.
    if (result == ESP_ERR_INVALID_STATE)
    {
        result =
            esp_task_wdt_reconfigure(&config);
    }

    if (result != ESP_OK)
    {
        return false;
    }

#else

    // Arduino-ESP32 2.x API'si saniye kullanir.
    const uint32_t timeoutSeconds =
        (HARDWARE_WATCHDOG_TIMEOUT_MS + 999U) /
        1000U;

    esp_err_t result =
        esp_task_wdt_init(
            timeoutSeconds,
            true
        );

    // Sistem tarafindan daha once baslatilmis olabilir.
    if (result != ESP_OK &&
        result != ESP_ERR_INVALID_STATE)
    {
        return false;
    }

#endif

    // nullptr mevcut Arduino loop task'ini temsil eder.
    const esp_err_t addResult =
        esp_task_wdt_add(nullptr);

    if (addResult == ESP_OK)
    {
        return true;
    }

    // Task zaten kayitliysa status ESP_OK dondurur.
    return esp_task_wdt_status(nullptr) == ESP_OK;
}