#include "MotionController.h"

#include <communication/Joystick.h>
#include <drivers/MotorDriver.h>

MotionController::MotionController()
  : _motorDriver(nullptr),
    _ready(false),
    _emergencyStopped(false),
    _maxPwm(200),
    _rampStep(8),
    _rampIntervalMs(20),
    _targetLeftPwm(0),
    _targetRightPwm(0),
    _currentLeftPwm(0),
    _currentRightPwm(0),
    _lastRampMs(0) {}

bool MotionController::begin(MotorDriver& motorDriver,
                             uint8_t maxPwmValue,
                             uint8_t rampStepValue,
                             uint16_t rampIntervalMsValue) {
  _motorDriver = &motorDriver;
  _maxPwm = (maxPwmValue == 0) ? 1 : maxPwmValue;
  _rampStep = (rampStepValue == 0) ? 1 : rampStepValue;
  _rampIntervalMs = (rampIntervalMsValue == 0) ? 1 : rampIntervalMsValue;

  _emergencyStopped = false;
  _targetLeftPwm = 0;
  _targetRightPwm = 0;
  _currentLeftPwm = 0;
  _currentRightPwm = 0;
  _lastRampMs = millis();

  // Motor surucusu daha once baslatilmadiysa varsayilan pinlerle baslatir.
  _ready = _motorDriver->ready() || _motorDriver->begin();

  if (_ready) {
    _motorDriver->stopDrive();
  }

  return _ready;
}

bool MotionController::drive(int16_t forwardPercent, int16_t turnPercent) {
  if (!_ready || _emergencyStopped) return false;

  forwardPercent = constrain(forwardPercent, static_cast<int16_t>(-100), static_cast<int16_t>(100));
  turnPercent = constrain(turnPercent, static_cast<int16_t>(-100), static_cast<int16_t>(100));

  int32_t forwardPwm = map(forwardPercent, -100, 100, -_maxPwm, _maxPwm);
  int32_t turnPwm = map(turnPercent, -100, 100, -_maxPwm, _maxPwm);

  int32_t left = forwardPwm + turnPwm;
  int32_t right = forwardPwm - turnPwm;

  // Sol ve sag oranini bozmadan en buyuk PWM degerine sigdirir.
  int32_t largest = abs(left);
  int32_t rightMagnitude = abs(right);
  if (rightMagnitude > largest) largest = rightMagnitude;

  if (largest > _maxPwm && largest > 0) {
    left = (left * _maxPwm) / largest;
    right = (right * _maxPwm) / largest;
  }

  return setTargetPwm(static_cast<int16_t>(left), static_cast<int16_t>(right));
}

bool MotionController::driveFromJoystick(const Joystick& joystick) {
  if (!_ready || _emergencyStopped) return false;

  if (joystick.timedOut()) {
    stop();
    return false;
  }

  // Gaz kolu kullanilmiyorsa Joystick sinifi yuzde 100 kabul eder.
  uint8_t allowedPwm = joystick.throttlePwm(0, _maxPwm);
  DifferentialOutput output = joystick.differential(allowedPwm);

  return setTargetPwm(output.leftPwm, output.rightPwm);
}

bool MotionController::setTargetPwm(int16_t leftPwm, int16_t rightPwm) {
  if (!_ready || _emergencyStopped) return false;

  _targetLeftPwm = limitPwm(leftPwm);
  _targetRightPwm = limitPwm(rightPwm);
  return true;
}

void MotionController::update() {
  if (!_ready || _motorDriver == nullptr) return;

  if (_emergencyStopped) {
    _motorDriver->stopDrive();
    return;
  }

  uint32_t now = millis();
  if ((now - _lastRampMs) < _rampIntervalMs) return;

  _lastRampMs = now;

  int16_t nextLeft = approach(_currentLeftPwm, _targetLeftPwm, _rampStep);
  int16_t nextRight = approach(_currentRightPwm, _targetRightPwm, _rampStep);

  if (nextLeft == _currentLeftPwm && nextRight == _currentRightPwm) return;

  if (_motorDriver->setDriveSpeeds(nextLeft, nextRight)) {
    _currentLeftPwm = nextLeft;
    _currentRightPwm = nextRight;
  }
}

void MotionController::softStop() {
  if (!_ready || _emergencyStopped) return;

  _targetLeftPwm = 0;
  _targetRightPwm = 0;
}

void MotionController::stop() {
  _targetLeftPwm = 0;
  _targetRightPwm = 0;
  _currentLeftPwm = 0;
  _currentRightPwm = 0;

  if (_motorDriver != nullptr) {
    _motorDriver->stopDrive();
  }
}

void MotionController::emergencyStop() {
  _emergencyStopped = true;
  stop();
}

void MotionController::clearEmergencyStop() {
  _emergencyStopped = false;
  _lastRampMs = millis();
}

bool MotionController::ready() const {
  return _ready;
}

bool MotionController::emergencyStopped() const {
  return _emergencyStopped;
}

int16_t MotionController::targetLeftPwm() const {
  return _targetLeftPwm;
}

int16_t MotionController::targetRightPwm() const {
  return _targetRightPwm;
}

int16_t MotionController::currentLeftPwm() const {
  return _currentLeftPwm;
}

int16_t MotionController::currentRightPwm() const {
  return _currentRightPwm;
}

uint8_t MotionController::maxPwm() const {
  return _maxPwm;
}

int16_t MotionController::limitPwm(int32_t value) const {
  if (value > _maxPwm) return _maxPwm;
  if (value < -static_cast<int32_t>(_maxPwm)) return -static_cast<int16_t>(_maxPwm);
  return static_cast<int16_t>(value);
}

int16_t MotionController::approach(int16_t current, int16_t target, uint8_t step) {
  if (current < target) {
    int32_t next = static_cast<int32_t>(current) + step;
    if (next > target) next = target;
    return static_cast<int16_t>(next);
  }

  if (current > target) {
    int32_t next = static_cast<int32_t>(current) - step;
    if (next < target) next = target;
    return static_cast<int16_t>(next);
  }

  return current;
}
