#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>
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

// Yarismada kullanilan sabit sehirler.
enum class City : uint8_t
{
    NONE,
    AYDIN,
    NIGDE,
    SINOP,
    TOKAT,
    ELAZIG
};

// Sehir enum degerini yazdirilabilir metne cevirir.
inline const char* cityToString(City city)
{
    switch (city)
    {
        case City::AYDIN:
            return "AYDIN";

        case City::NIGDE:
            return "NIGDE";

        case City::SINOP:
            return "SINOP";

        case City::TOKAT:
            return "TOKAT";

        case City::ELAZIG:
            return "ELAZIG";

        case City::NONE:
        default:
            return "NONE";
    }
}

struct DifferentialOutput
{
    int16_t leftPwm;
    int16_t rightPwm;
};

struct JoystickPacket
{
    int16_t xPercent;         // Sag-sol ekseni
    int16_t yPercent;         // Ileri-geri ekseni
    int16_t twistPercent;     // Sap cevirme ekseni
    uint8_t throttlePercent;  // Gaz kolu yuzdesi
    int16_t hatAngle;         // Hat switch acisi
    uint32_t buttons;         // Buton bit maskesi
    uint32_t timestampMs;     // Son paket zamani
    bool valid;               // Paket gecerli mi
};

// Tum sistemde kullanilan ortak slot modeli.
struct SlotInfo
{
    bool occupied;
    String uid;
    City city;
};

#endif