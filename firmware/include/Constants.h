#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>

constexpr uint8_t SLOT_COUNT = 5;

// Timing
constexpr uint16_t WATCHDOG_TIMEOUT_MS = 1000;
constexpr uint16_t CONTROL_LOOP_INTERVAL_MS = 20;
constexpr uint16_t DISPLAY_UPDATE_INTERVAL_MS = 250;

// Joystick
constexpr int JOYSTICK_MIN_VALUE = -100;
constexpr int JOYSTICK_MAX_VALUE = 100;

// Motor
constexpr int MOTOR_MIN_PWM = 0;
constexpr int MOTOR_MAX_PWM = 255;

// Battery (3S LiPo)
constexpr float BATTERY_FULL_VOLTAGE    = 12.6f;
constexpr float BATTERY_NOMINAL_VOLTAGE = 11.1f;
constexpr float LOW_BATTERY_VOLTAGE     = 10.5f;

#endif