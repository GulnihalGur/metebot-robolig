#pragma once

#include <Arduino.h>
#include <drivers/ServoDriver.h>

// Robot kolundaki eklemleri belirtir.
enum class ServoJoint : uint8_t {
  BASE = 0,
  SHOULDER = 1,
  ELBOW = 2,
  WRIST = 3,
  GRIPPER = 4
};

// Bir eklemin hareket ayarlarini tutar.
struct JointConfig {
  uint8_t servoIndex;
  uint8_t minAngle;
  uint8_t maxAngle;
  uint8_t homeAngle;
  uint16_t speedDegPerSec;
};

// ServoDriver uzerinden eklemleri yonetir.
class ServoManager {
public:
  static constexpr uint8_t JOINT_COUNT = 5;

  ServoManager();

  // ServoDriver ve eklem ayarlarini kaydeder.
  bool begin(ServoDriver& servoDriver,
             const JointConfig* jointConfigs,
             uint8_t jointCount = JOINT_COUNT);

  // Ekleme guvenli hedef aci gonderir.
  bool setTargetAngle(ServoJoint joint, int16_t angle);

  // Eklemi beklemeden belirtilen aciya getirir.
  bool writeNow(ServoJoint joint, int16_t angle);

  // Eklemi belirtilen miktarda hareket ettirir.
  bool changeTargetAngle(ServoJoint joint, int16_t angleChange);

  // Tum eklemleri baslangic acilarina yollar.
  void moveHome();

  // Hedefleri mevcut acilarda sabitler.
  void stop();

  // ServoDriver hareketlerini gunceller.
  void update();

  // Eklem acilarini dondurur.
  uint8_t angle(ServoJoint joint) const;
  uint8_t target(ServoJoint joint) const;

  bool ready() const;

private:
  ServoDriver* _servoDriver;
  JointConfig _joints[JOINT_COUNT];
  uint8_t _jointCount;
  bool _ready;

  bool validJoint(ServoJoint joint) const;
  uint8_t jointToIndex(ServoJoint joint) const;
  uint8_t limitAngle(int16_t angle, const JointConfig& config) const;
};
