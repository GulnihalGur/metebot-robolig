#include "TaskScheduler.h"

void TaskScheduler::begin()
{
    taskCount = 0;

    for (uint8_t i = 0; i < Constants::MAX_TASK_COUNT; ++i)
    {
        tasks[i] = Task{};
    }
}

void TaskScheduler::update()
{
    const uint32_t currentTimeMs = millis();

    for (uint8_t i = 0; i < Constants::MAX_TASK_COUNT; ++i)
    {
        Task& task = tasks[i];

        if (!task.occupied ||
            !task.enabled ||
            task.callback == nullptr)
        {
            continue;
        }

        if (currentTimeMs - task.lastRunMs >= task.intervalMs)
        {
            task.lastRunMs = currentTimeMs;
            task.callback();
        }
    }
}

int8_t TaskScheduler::addTask(
    TaskCallback callback,
    uint32_t intervalMs,
    bool runImmediately
)
{
    if (callback == nullptr || intervalMs == 0)
    {
        return INVALID_TASK_ID;
    }

    for (uint8_t i = 0; i < Constants::MAX_TASK_COUNT; ++i)
    {
        if (tasks[i].occupied)
        {
            continue;
        }

        tasks[i].callback = callback;
        tasks[i].intervalMs = intervalMs;
        tasks[i].enabled = true;
        tasks[i].occupied = true;

        const uint32_t currentTimeMs = millis();

        tasks[i].lastRunMs = runImmediately
            ? currentTimeMs - intervalMs
            : currentTimeMs;

        ++taskCount;

        return static_cast<int8_t>(i);
    }

    return INVALID_TASK_ID;
}

bool TaskScheduler::setTaskEnabled(
    uint8_t taskId,
    bool enabled
)
{
    if (!isValidTaskId(taskId))
    {
        return false;
    }

    tasks[taskId].enabled = enabled;

    if (enabled)
    {
        tasks[taskId].lastRunMs = millis();
    }

    return true;
}

bool TaskScheduler::resetTask(uint8_t taskId)
{
    if (!isValidTaskId(taskId))
    {
        return false;
    }

    tasks[taskId].lastRunMs = millis();

    return true;
}

bool TaskScheduler::removeTask(uint8_t taskId)
{
    if (!isValidTaskId(taskId))
    {
        return false;
    }

    tasks[taskId] = Task{};

    if (taskCount > 0)
    {
        --taskCount;
    }

    return true;
}

uint8_t TaskScheduler::getTaskCount() const
{
    return taskCount;
}

bool TaskScheduler::isValidTaskId(uint8_t taskId) const
{
    return taskId < Constants::MAX_TASK_COUNT &&
           tasks[taskId].occupied;
}