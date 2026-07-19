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
constexpr float BATTERY_FULL_VOLTAGE     = 12.6f;
constexpr float BATTERY_NOMINAL_VOLTAGE  = 11.1f;
constexpr float LOW_BATTERY_VOLTAGE      = 10.5f;

namespace Constants
{
    constexpr uint8_t MAX_TASK_COUNT = 12;

    // Joystick butonlari bit maskesinde 0 tabanli indeks kullanir.
    namespace JoystickButtons
    {
        constexpr uint8_t GRIPPER_OPEN = 0;
        constexpr uint8_t GRIPPER_CLOSE = 1;
        constexpr uint8_t MODE_TOGGLE = 2;
        constexpr uint8_t ZIPLINE_CONFIRM = 3;
    }
}

// Mod tusunun kiskac tuslariyla ayni olmasini derleme asamasinda engeller.
static_assert(
    Constants::JoystickButtons::MODE_TOGGLE !=
        Constants::JoystickButtons::GRIPPER_OPEN,
    "MODE_TOGGLE and GRIPPER_OPEN buttons must be different."
);

static_assert(
    Constants::JoystickButtons::MODE_TOGGLE !=
        Constants::JoystickButtons::GRIPPER_CLOSE,
    "MODE_TOGGLE and GRIPPER_CLOSE buttons must be different."
);

static_assert(
    Constants::JoystickButtons::GRIPPER_OPEN !=
        Constants::JoystickButtons::GRIPPER_CLOSE,
    "Gripper open and close buttons must be different."
);

static_assert(
    Constants::JoystickButtons::ZIPLINE_CONFIRM !=
        Constants::JoystickButtons::MODE_TOGGLE,
    "ZIPLINE_CONFIRM and MODE_TOGGLE buttons must be different."
);

static_assert(
    Constants::JoystickButtons::ZIPLINE_CONFIRM !=
        Constants::JoystickButtons::GRIPPER_OPEN,
    "ZIPLINE_CONFIRM and GRIPPER_OPEN buttons must be different."
);

static_assert(
    Constants::JoystickButtons::ZIPLINE_CONFIRM !=
        Constants::JoystickButtons::GRIPPER_CLOSE,
    "ZIPLINE_CONFIRM and GRIPPER_CLOSE buttons must be different."
);

#endif
