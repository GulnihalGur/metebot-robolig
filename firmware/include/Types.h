#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

enum class RobotState
{
    IDLE,
    MANUAL,
    PICKUP,  
    DELIVERY,
    ZIPLINE,
    FINISH,
    FAIL_SAFE
};

enum class ErrorCode
{
    NONE,
    COMMUNICATION_LOST,
    RFID_READ_FAILED,
    MOTOR_FAULT,
    SERVO_FAULT,
    LOW_BATTERY,
    UNKNOWN_ERROR
};

enum class City
{
    NONE,
    AYDIN,
    NIGDE,
    SINOP,
    TOKAT,
    ELAZIG
};

struct JoystickData
{
    int x;
    int y;
    bool buttonPressed;
    bool isConnected;
};

struct SlotData
{
    uint8_t slotId;
    bool isOccupied;
    uint32_t rfidUid;
    City city;
};

#endif