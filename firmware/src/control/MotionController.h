#pragma once

#include <Arduino.h>

class MotorDriver;
class Joystick;

// Robotun 6 surus motorunu sol ve sag grup olarak yonetir.
class MotionController {
public:
  MotionController();

  // MotorDriver hazir degilse burada baslatilir.
  bool begin(MotorDriver& motorDriver,
             uint8_t maxPwm = 200,
             uint8_t rampStep = 8,
             uint16_t rampIntervalMs = 20);

  // Ileri ve donus degerleri -100 ile 100 arasindadir.
  bool drive(int16_t forwardPercent, int16_t turnPercent);

  // Joystick verisini diferansiyel suruse uygular.
  bool driveFromJoystick(const Joystick& joystick);

  // Sol ve sag hedef PWM degerlerini ayarlar.
  bool setTargetPwm(int16_t leftPwm, int16_t rightPwm);

  // Hedef hiza bloklamadan yumusak gecis yapar.
  void update();

  // Hedefleri sifira indirir, yavaslama update ile yapilir.
  void softStop();

  // Motorlari beklemeden durdurur.
  void stop();

  // Acil durdurma kilidini etkinlestirir.
  void emergencyStop();

  // Acil durdurma kilidini kaldirir.
  void clearEmergencyStop();

  bool ready() const;
  bool emergencyStopped() const;

  int16_t targetLeftPwm() const;
  int16_t targetRightPwm() const;
  int16_t currentLeftPwm() const;
  int16_t currentRightPwm() const;
  uint8_t maxPwm() const;

private:
  MotorDriver* _motorDriver;
  bool _ready;
  bool _emergencyStopped;

  uint8_t _maxPwm;
  uint8_t _rampStep;
  uint16_t _rampIntervalMs;

  int16_t _targetLeftPwm;
  int16_t _targetRightPwm;
  int16_t _currentLeftPwm;
  int16_t _currentRightPwm;

  uint32_t _lastRampMs;

  int16_t limitPwm(int32_t value) const;
  static int16_t approach(int16_t current, int16_t target, uint8_t step);
};
