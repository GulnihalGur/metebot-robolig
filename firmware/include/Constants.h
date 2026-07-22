#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>

// Robot üzerindeki toplam yük slotu sayısı
constexpr uint8_t SLOT_COUNT = 5;

// Zamanlama değerleri
constexpr uint16_t SOFTWARE_WATCHDOG_TIMEOUT_MS = 1000;
constexpr uint32_t HARDWARE_WATCHDOG_TIMEOUT_MS = 5000;

constexpr uint16_t CONTROL_LOOP_INTERVAL_MS = 20;
constexpr uint16_t DISPLAY_UPDATE_INTERVAL_MS = 250;

// Joystick eksen sınırları
constexpr int JOYSTICK_MIN_VALUE = -100;
constexpr int JOYSTICK_MAX_VALUE = 100;

// Motor PWM sınırları
constexpr int MOTOR_MIN_PWM = 0;
constexpr int MOTOR_MAX_PWM = 255;

// Batarya değerleri (3S LiPo)
constexpr float BATTERY_FULL_VOLTAGE = 12.6f;
constexpr float BATTERY_NOMINAL_VOLTAGE = 11.1f;
constexpr float LOW_BATTERY_VOLTAGE = 10.5f;

namespace Constants
{
    // Görev zamanlayıcısına eklenebilecek en fazla görev sayısı
    constexpr uint8_t MAX_TASK_COUNT = 12;

    // Joystick butonlarının bit maskesi üzerindeki indeksleri
    namespace JoystickButtons
    {
        constexpr uint8_t GRIPPER_OPEN = 0;
        constexpr uint8_t GRIPPER_CLOSE = 1;
        constexpr uint8_t MODE_TOGGLE = 2;
        constexpr uint8_t ZIPLINE_CONFIRM = 3;
    }
}

// Mod değiştirme butonunun kıskaç açma butonuyla aynı olmamasını denetler
static_assert(
    Constants::JoystickButtons::MODE_TOGGLE !=
        Constants::JoystickButtons::GRIPPER_OPEN,
    "MODE_TOGGLE and GRIPPER_OPEN buttons must be different."
);

// Mod değiştirme butonunun kıskaç kapatma butonuyla aynı olmamasını denetler
static_assert(
    Constants::JoystickButtons::MODE_TOGGLE !=
        Constants::JoystickButtons::GRIPPER_CLOSE,
    "MODE_TOGGLE and GRIPPER_CLOSE buttons must be different."
);

// Kıskaç açma ve kapatma butonlarının farklı olmasını denetler
static_assert(
    Constants::JoystickButtons::GRIPPER_OPEN !=
        Constants::JoystickButtons::GRIPPER_CLOSE,
    "Gripper open and close buttons must be different."
);

// Zipline onay butonunun mod değiştirme butonuyla aynı olmamasını denetler
static_assert(
    Constants::JoystickButtons::ZIPLINE_CONFIRM !=
        Constants::JoystickButtons::MODE_TOGGLE,
    "ZIPLINE_CONFIRM and MODE_TOGGLE buttons must be different."
);

// Zipline onay butonunun kıskaç açma butonuyla aynı olmamasını denetler
static_assert(
    Constants::JoystickButtons::ZIPLINE_CONFIRM !=
        Constants::JoystickButtons::GRIPPER_OPEN,
    "ZIPLINE_CONFIRM and GRIPPER_OPEN buttons must be different."
);

// Zipline onay butonunun kıskaç kapatma butonuyla aynı olmamasını denetler
static_assert(
    Constants::JoystickButtons::ZIPLINE_CONFIRM !=
        Constants::JoystickButtons::GRIPPER_CLOSE,
    "ZIPLINE_CONFIRM and GRIPPER_CLOSE buttons must be different."
);

#endif