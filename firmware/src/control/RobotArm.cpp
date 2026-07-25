#include "RobotArm.h"

RobotArm::RobotArm()
  : _joystick(nullptr),
    _servoManager(nullptr),
    _lastControlMs(0),
    _ready(false),
    _timedOut(true),
    _active(false),
    _baseAngleAccumulator(0),
    _shoulderAngleAccumulator(0),
    _elbowAngleAccumulator(0),
    _wristAngleAccumulator(0),
    _gripperAngleAccumulator(0) {}

bool RobotArm::begin(Joystick& joystick,
                     ServoManager& servoManager,
                     const RobotArmConfig& config) {
  if (!servoManager.ready() ||
      config.joystickTimeoutMs == 0 ||
      config.minControlSpeedDegPerSec == 0 ||
      config.minControlSpeedDegPerSec >
        config.maxControlSpeedDegPerSec ||
      config.gripperControlSpeedDegPerSec == 0 ||
      config.gripperOpenButton >= 32 ||
      config.gripperCloseButton >= 32 ||
      config.gripperOpenButton ==
        config.gripperCloseButton) {
    return false;
  }

  _joystick = &joystick;
  _servoManager = &servoManager;
  _config = config;

  _lastControlMs = millis();
  _timedOut = true;
  _active = false;
  resetAngleAccumulators();
  _ready = true;

  return true;
}

void RobotArm::update() {
  if (!_ready) return;

  // DRIVE modunda joystick eksenlerini kola uygulamaz.
  if (!_active) {
    _servoManager->update();
    return;
  }

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


void RobotArm::setActive(bool activeValue) {
  if (!_ready || _active == activeValue) return;

  _active = activeValue;
  _lastControlMs = millis();
  _timedOut = true;
  resetAngleAccumulators();

  // Kol kontrolu kapanirken hedefleri mevcut acilarda tutar.
  if (!_active) {
    stop();
  }
}

bool RobotArm::active() const {
  return _active;
}

void RobotArm::moveHome() {
  if (!_ready) return;

  resetAngleAccumulators();
  _servoManager->moveHome();
}

void RobotArm::stop() {
  if (!_ready) return;

  resetAngleAccumulators();
  _servoManager->stop();
}

bool RobotArm::ready() const {
  return _ready;
}

int16_t RobotArm::calculateAngleChange(
  int16_t axisPercent,
  uint16_t speedDegPerSec,
  uint32_t elapsedMs,
  bool inverted,
  int64_t& angleAccumulator
) {
  // Joystick merkeze dondugunde eski kesirli hareketi tasimaz.
  if (axisPercent == 0) {
    angleAccumulator = 0;
    return 0;
  }

  if (elapsedMs == 0) return 0;

  int32_t direction = inverted ? -axisPercent : axisPercent;

  // Yon degistiginde onceki yone ait kesirli hareketi temizler.
  if ((angleAccumulator > 0 && direction < 0) ||
      (angleAccumulator < 0 && direction > 0)) {
    angleAccumulator = 0;
  }

  // 100000 birim 1 dereceyi temsil eder.
  // int64_t kullanimi uzun surelerde carpma tasmasini engeller.
  int64_t movementUnits =
    static_cast<int64_t>(direction) *
    static_cast<int64_t>(speedDegPerSec) *
    static_cast<int64_t>(elapsedMs);

  angleAccumulator += movementUnits;

  // Birikim tam dereceye ulasmadiysa servo hedefini degistirmez.
  if (angleAccumulator > -ANGLE_UNITS_PER_DEGREE &&
      angleAccumulator < ANGLE_UNITS_PER_DEGREE) {
    return 0;
  }

  int64_t wholeDegrees =
    angleAccumulator / ANGLE_UNITS_PER_DEGREE;

  // Uygulanan tam dereceleri cikarip yalnizca kesirli kalani saklar.
  angleAccumulator %= ANGLE_UNITS_PER_DEGREE;

  // Donus tipinin sinirlarini asan anormal gecikmeleri guvenli sinirlar.
  if (wholeDegrees > INT16_MAX) return INT16_MAX;
  if (wholeDegrees < INT16_MIN) return INT16_MIN;

  return static_cast<int16_t>(wholeDegrees);
}

void RobotArm::resetAngleAccumulators() {
  _baseAngleAccumulator = 0;
  _shoulderAngleAccumulator = 0;
  _elbowAngleAccumulator = 0;
  _wristAngleAccumulator = 0;
  _gripperAngleAccumulator = 0;
}


int16_t RobotArm::wristAxisPercent(int16_t hatAngle) {
  // Hat merkezdeyse bilek durur.
  if (hatAngle < 0) return 0;

  // Yukari ve yukari capraz yonler bilegi bir yone hareket ettirir.
  if (hatAngle >= 315 || hatAngle <= 45) return 100;

  // Asagi ve asagi capraz yonler bilegi ters yone hareket ettirir.
  if (hatAngle >= 135 && hatAngle <= 225) return -100;

  // Yalnizca sag veya sol secildiginde bilek hareket etmez.
  return 0;
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

  // Twist hareketi robot kolunun tabanini (base) dondurur.
  int16_t baseChange = calculateAngleChange(
    packet.twistPercent,
    controlSpeed,
    elapsedMs,
    _config.invertBase,
    _baseAngleAccumulator
  );

  // X ekseni omuz eklemini sag/sol yonunde kontrol eder.
  int16_t shoulderChange = calculateAngleChange(
    packet.xPercent,
    controlSpeed,
    elapsedMs,
    _config.invertShoulder,
    _shoulderAngleAccumulator
  );

  // Y ekseni dirsek eklemini ileri/geri kontrol eder.
  int16_t elbowChange = calculateAngleChange(
    packet.yPercent,
    controlSpeed,
    elapsedMs,
    _config.invertElbow,
    _elbowAngleAccumulator
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


  // Joystick uzerindeki hat switch yukari-asagi hareketi bilegi kontrol eder.
  int16_t wristChange = calculateAngleChange(
    wristAxisPercent(packet.hatAngle),
    controlSpeed,
    elapsedMs,
    _config.invertWrist,
    _wristAngleAccumulator
  );

  if (wristChange != 0) {
    _servoManager->changeTargetAngle(
      ServoJoint::WRIST,
      wristChange
    );
  }

  // Kiskac butonlari artik sabit aciya gitmez.
  // Buton basili tutuldugu surece hedef aci yavas ve kademeli degisir.
  const bool openPressed = _joystick->buttonPressed(
    _config.gripperOpenButton
  );
  const bool closePressed = _joystick->buttonPressed(
    _config.gripperCloseButton
  );

  int16_t gripperAxisPercent = 0;

  // Iki buton ayni anda basilirsa celisen komut nedeniyle hareket etmez.
  if (openPressed != closePressed) {
    gripperAxisPercent = openPressed ? -100 : 100;
  }

  int16_t gripperChange = calculateAngleChange(
    gripperAxisPercent,
    _config.gripperControlSpeedDegPerSec,
    elapsedMs,
    false,
    _gripperAngleAccumulator
  );

  if (gripperChange != 0) {
    _servoManager->changeTargetAngle(
      ServoJoint::GRIPPER,
      gripperChange
    );
  }
}
