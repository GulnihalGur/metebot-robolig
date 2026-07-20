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

struct DifferentialOutput {
  int16_t leftPwm;
  int16_t rightPwm;
};

struct JoystickPacket {
  int16_t xPercent;          // Sag sol ekseni
  int16_t yPercent;          // Ileri geri ekseni
  int16_t twistPercent;      // Sap cevirme ekseni
  uint8_t throttlePercent;   // Gaz kolu yuzdesi
  int16_t hatAngle;          // Hat switch acisi
  uint32_t buttons;          // Buton bit maskesi
  uint32_t timestampMs;      // Son paket zamani
  bool valid;                // Paket gecerli mi
};

struct SlotData
{
    uint8_t slotId;
    bool isOccupied;
    uint32_t rfidUid;
    City city;
};

#endif
