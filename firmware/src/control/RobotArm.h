#pragma once

#include <Arduino.h>
#include <communication/Joystick.h>
#include <control/ServoManager.h>

// Joystick ile kol kontrol ayarlarini tutar.
struct RobotArmConfig {
  uint16_t joystickTimeoutMs;

  uint8_t minControlSpeedDegPerSec;
  uint8_t maxControlSpeedDegPerSec;

  uint8_t gripperOpenAngle;
  uint8_t gripperClosedAngle;

  uint8_t gripperOpenButton;
  uint8_t gripperCloseButton;

  bool invertBase;
  bool invertShoulder;
  bool invertElbow;
};

// Joystick verisini eklem acilarina cevirir.
class RobotArm {
public:
  RobotArm();

  // Gerekli siniflari ve ayarlari kaydeder.
  bool begin(Joystick& joystick,
             ServoManager& servoManager,
             const RobotArmConfig& config);

  // Joystick verisine gore hedef acilari hesaplar.
  void update();

  // Kolun joystick kontrolunu etkinlestirir veya durdurur.
  void setActive(bool active);

  bool active() const;

  // Kolu baslangic konumuna yollar.
  void moveHome();

  // Kol hareketini mevcut konumda durdurur.
  void stop();

  bool ready() const;

private:
  Joystick* _joystick;
  ServoManager* _servoManager;
  RobotArmConfig _config;

  uint32_t _lastControlMs;
  bool _ready;
  bool _timedOut;
  bool _active;

  // Joystick yuzdesini aci degisimine cevirir.
  int16_t calculateAngleChange(int16_t axisPercent,
                               uint16_t speedDegPerSec,
                               uint32_t elapsedMs,
                               bool inverted) const;

  // Gaz koluna gore kontrol hizini hesaplar.
  uint16_t calculateControlSpeed(
    uint8_t throttlePercent
  ) const;

  // Joystick verisini eklemlere uygular.
  void applyJoystick(const JoystickPacket& packet,
                     uint32_t elapsedMs);
};
