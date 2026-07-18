#include "ServoManager.h"

ServoManager::ServoManager()
  : _servoDriver(nullptr),
    _jointCount(0),
    _ready(false) {}

bool ServoManager::begin(ServoDriver& servoDriver,
                         const JointConfig* jointConfigs,
                         uint8_t jointCount) {
  if (!servoDriver.ready() ||
      jointConfigs == nullptr ||
      jointCount != JOINT_COUNT) {
    return false;
  }

  // Tum eklem ayarlarini kontrol eder.
  for (uint8_t i = 0; i < jointCount; ++i) {
    const JointConfig& config = jointConfigs[i];

    if (config.servoIndex >= servoDriver.count() ||
        config.minAngle >= config.maxAngle ||
        config.homeAngle < config.minAngle ||
        config.homeAngle > config.maxAngle ||
        config.speedDegPerSec == 0) {
      return false;
    }
  }

  _servoDriver = &servoDriver;
  _jointCount = jointCount;

  // Eklem ayarlarini yerel olarak saklar.
  for (uint8_t i = 0; i < _jointCount; ++i) {
    _joints[i] = jointConfigs[i];
  }

  _ready = true;
  return true;
}

bool ServoManager::setTargetAngle(ServoJoint joint, int16_t angleValue) {
  if (!_ready || !validJoint(joint)) return false;

  uint8_t jointIndex = jointToIndex(joint);
  const JointConfig& config = _joints[jointIndex];

  uint8_t safeAngle = limitAngle(angleValue, config);

  return _servoDriver->moveTo(
    config.servoIndex,
    safeAngle,
    config.speedDegPerSec
  );
}

bool ServoManager::writeNow(ServoJoint joint, int16_t angleValue) {
  if (!_ready || !validJoint(joint)) return false;

  uint8_t jointIndex = jointToIndex(joint);
  const JointConfig& config = _joints[jointIndex];

  uint8_t safeAngle = limitAngle(angleValue, config);

  return _servoDriver->writeNow(
    config.servoIndex,
    safeAngle
  );
}

bool ServoManager::changeTargetAngle(ServoJoint joint,
                                     int16_t angleChange) {
  if (!_ready || !validJoint(joint)) return false;

  int16_t newTarget =
    static_cast<int16_t>(target(joint)) + angleChange;

  return setTargetAngle(joint, newTarget);
}

void ServoManager::moveHome() {
  if (!_ready) return;

  // Her eklemi kendi baslangic acisina yollar.
  for (uint8_t i = 0; i < _jointCount; ++i) {
    setTargetAngle(
      static_cast<ServoJoint>(i),
      _joints[i].homeAngle
    );
  }
}

void ServoManager::stop() {
  if (!_ready) return;

  // Yeni hedefleri mevcut acilara esitler.
  for (uint8_t i = 0; i < _jointCount; ++i) {
    ServoJoint joint = static_cast<ServoJoint>(i);
    setTargetAngle(joint, angle(joint));
  }
}

void ServoManager::update() {
  if (!_ready) return;

  _servoDriver->update();
}

uint8_t ServoManager::angle(ServoJoint joint) const {
  if (!_ready || !validJoint(joint)) return 0;

  const JointConfig& config =
    _joints[jointToIndex(joint)];

  return _servoDriver->angle(config.servoIndex);
}

uint8_t ServoManager::target(ServoJoint joint) const {
  if (!_ready || !validJoint(joint)) return 0;

  const JointConfig& config =
    _joints[jointToIndex(joint)];

  return _servoDriver->target(config.servoIndex);
}

bool ServoManager::ready() const {
  return _ready;
}

bool ServoManager::validJoint(ServoJoint joint) const {
  return jointToIndex(joint) < _jointCount;
}

uint8_t ServoManager::jointToIndex(ServoJoint joint) const {
  return static_cast<uint8_t>(joint);
}

uint8_t ServoManager::limitAngle(
  int16_t angleValue,
  const JointConfig& config
) const {
  return static_cast<uint8_t>(
    constrain(
      angleValue,
      static_cast<int16_t>(config.minAngle),
      static_cast<int16_t>(config.maxAngle)
    )
  );
}
