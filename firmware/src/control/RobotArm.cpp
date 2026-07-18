#include "RobotArm.h"

RobotArm::RobotArm()
  : _joystick(nullptr),
    _servoManager(nullptr),
    _lastControlMs(0),
    _ready(false),
    _timedOut(true) {}

bool RobotArm::begin(Joystick& joystick,
                     ServoManager& servoManager,
                     const RobotArmConfig& config) {
  if (!servoManager.ready() ||
      config.joystickTimeoutMs == 0 ||
      config.minControlSpeedDegPerSec == 0 ||
      config.minControlSpeedDegPerSec >
        config.maxControlSpeedDegPerSec) {
    return false;
  }

  _joystick = &joystick;
  _servoManager = &servoManager;
  _config = config;

  _lastControlMs = millis();
  _timedOut = true;
  _ready = true;

  return true;
}

void RobotArm::update() {
  if (!_ready) return;

  uint32_t now = millis();
  uint32_t elapsedMs = now - _lastControlMs;
  _lastControlMs = now;

  // Joystick verisi eskiyse kolu durdurur.
  if (_joystick->timedOut(_config.joystickTimeoutMs)) {
    if (!_timedOut) {
      stop();
      _timedOut = true;
    }

    _servoManager->update();
    return;
  }

  _timedOut = false;

  // Son joystick paketini kullanir.
  applyJoystick(
    _joystick->packet(),
    elapsedMs
  );

  // Servo hareketlerini bloklamadan ilerletir.
  _servoManager->update();
}

void RobotArm::moveHome() {
  if (!_ready) return;

  _servoManager->moveHome();
}

void RobotArm::stop() {
  if (!_ready) return;

  _servoManager->stop();
}

bool RobotArm::ready() const {
  return _ready;
}

int16_t RobotArm::calculateAngleChange(
  int16_t axisPercent,
  uint16_t speedDegPerSec,
  uint32_t elapsedMs,
  bool inverted
) const {
  if (axisPercent == 0 || elapsedMs == 0) return 0;

  int32_t direction = inverted ? -axisPercent : axisPercent;

  // Yuzde, hiz ve gecen sureye gore aci hesaplar.
  int32_t numerator =
    direction *
    static_cast<int32_t>(speedDegPerSec) *
    static_cast<int32_t>(elapsedMs);

  int16_t angleChange =
    static_cast<int16_t>(numerator / 100000L);

  // Dusuk hizlarda hareketin kaybolmasini engeller.
  if (angleChange == 0) {
    angleChange = direction > 0 ? 1 : -1;
  }

  return angleChange;
}

uint16_t RobotArm::calculateControlSpeed(
  uint8_t throttlePercent
) const {
  uint8_t safeThrottle =
    constrain(throttlePercent, 0, 100);

  return static_cast<uint16_t>(
    map(
      safeThrottle,
      0,
      100,
      _config.minControlSpeedDegPerSec,
      _config.maxControlSpeedDegPerSec
    )
  );
}

void RobotArm::applyJoystick(
  const JoystickPacket& packet,
  uint32_t elapsedMs
) {
  if (!packet.valid) return;

  uint16_t controlSpeed =
    calculateControlSpeed(packet.throttlePercent);

  // X ekseni taban servosunu kontrol eder.
  int16_t baseChange = calculateAngleChange(
    packet.xPercent,
    controlSpeed,
    elapsedMs,
    _config.invertBase
  );

  // Y ekseni omuz servosunu kontrol eder.
  int16_t shoulderChange = calculateAngleChange(
    packet.yPercent,
    controlSpeed,
    elapsedMs,
    _config.invertShoulder
  );

  // Twist ekseni dirsek servosunu kontrol eder.
  int16_t elbowChange = calculateAngleChange(
    packet.twistPercent,
    controlSpeed,
    elapsedMs,
    _config.invertElbow
  );

  if (baseChange != 0) {
    _servoManager->changeTargetAngle(
      ServoJoint::BASE,
      baseChange
    );
  }

  if (shoulderChange != 0) {
    _servoManager->changeTargetAngle(
      ServoJoint::SHOULDER,
      shoulderChange
    );
  }

  if (elbowChange != 0) {
    _servoManager->changeTargetAngle(
      ServoJoint::ELBOW,
      elbowChange
    );
  }

  // Birinci buton kiskaci acar.
  if (_joystick->buttonPressed(
        _config.gripperOpenButton
      )) {
    _servoManager->setTargetAngle(
      ServoJoint::GRIPPER,
      _config.gripperOpenAngle
    );
  }

  // Ikinci buton kiskaci kapatir.
  if (_joystick->buttonPressed(
        _config.gripperCloseButton
      )) {
    _servoManager->setTargetAngle(
      ServoJoint::GRIPPER,
      _config.gripperClosedAngle
    );
  }
}
