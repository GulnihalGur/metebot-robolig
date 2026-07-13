#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H

#include <Arduino.h>
#include "Constants.h"

class TaskScheduler
{
public:
    using TaskCallback = void (*)();

    static constexpr int8_t INVALID_TASK_ID = -1;

    void begin();
    void update();

    int8_t addTask(
        TaskCallback callback,
        uint32_t intervalMs,
        bool runImmediately = false
    );

    bool setTaskEnabled(uint8_t taskId, bool enabled);
    bool resetTask(uint8_t taskId);
    bool removeTask(uint8_t taskId);

    uint8_t getTaskCount() const;

private:
    struct Task
    {
        TaskCallback callback = nullptr;
        uint32_t intervalMs = 0;
        uint32_t lastRunMs = 0;
        bool enabled = false;
        bool occupied = false;
    };

    Task tasks[Constants::MAX_TASK_COUNT];
    uint8_t taskCount = 0;

    bool isValidTaskId(uint8_t taskId) const;
};

#endif