#include "Diagnostics.h"

#include <math.h>
#include <stdio.h>

#include "Constants.h"
#include "Pins.h"
#include "RobotConfig.h"
#include "Version.h"
#include "communication/Joystick.h"
#include "control/MotionController.h"
#include "control/ServoManager.h"
#include "core/ErrorManager.h"
#include "drivers/LinearActuator.h"
#include "drivers/MotorDriver.h"
#include "drivers/OLED.h"
#include "drivers/PowerManager.h"
#include "drivers/RFID.h"
#include "drivers/ServoDriver.h"

Diagnostics::Diagnostics()
    : _motionController(nullptr), _motorDriver(nullptr), _linearActuator(nullptr),
      _rfid(nullptr), _oled(nullptr), _powerManager(nullptr), _servoDriver(nullptr),
      _servoManager(nullptr), _joystick(nullptr), _errorManager(nullptr),
      _reportStream(nullptr), _lastSummary{0, 0, 0, 0, 0, 0, 0},
      _ready(false), _runCounter(0) {}

bool Diagnostics::begin(MotionController& motionController,
                        MotorDriver& motorDriver,
                        LinearActuator* linearActuator,
                        RFID* rfid,
                        OLED* oled,
                        PowerManager* powerManager,
                        ServoDriver* servoDriver,
                        ServoManager* servoManager,
                        Joystick* joystick,
                        ErrorManager& errorManager,
                        Stream& reportStream) {
  _motionController = &motionController;
  _motorDriver = &motorDriver;
  _linearActuator = linearActuator;
  _rfid = rfid;
  _oled = oled;
  _powerManager = powerManager;
  _servoDriver = servoDriver;
  _servoManager = servoManager;
  _joystick = joystick;
  _errorManager = &errorManager;
  _reportStream = &reportStream;
  _ready = true;
  resetSummary();
  return true;
}

void Diagnostics::resetSummary() {
  _lastSummary.passed = 0;
  _lastSummary.warnings = 0;
  _lastSummary.failed = 0;
  _lastSummary.total = 0;
  _lastSummary.runNumber = _runCounter;
  _lastSummary.durationMs = 0;
  _lastSummary.completedAtMs = 0;
}

void Diagnostics::record(const char* name, Result result, const char* detail, bool verbose) {
  ++_lastSummary.total;
  if (result == Result::PASS) ++_lastSummary.passed;
  else if (result == Result::WARNING) ++_lastSummary.warnings;
  else ++_lastSummary.failed;

  if (!verbose || _reportStream == nullptr) return;
  _reportStream->print(F("DIAG,CHECK,"));
  _reportStream->print(name);
  _reportStream->print(',');
  _reportStream->print(resultName(result));
  _reportStream->print(',');
  _reportStream->println(detail);
}

void Diagnostics::printHeader() const {
  if (_reportStream == nullptr) return;
  _reportStream->println(F("DIAG,BEGIN,SAFE_DETAILED_CHECKS"));
  _reportStream->print(F("DIAG,META,PROJECT=")); _reportStream->print(Version::PROJECT_NAME);
  _reportStream->print(F(",VERSION=")); _reportStream->print(Version::SOFTWARE_VERSION);
  _reportStream->print(F(",BUILD=")); _reportStream->println(Version::BUILD_TYPE);
  _reportStream->print(F("DIAG,META,RUN=")); _reportStream->print(_runCounter);
  _reportStream->print(F(",UPTIME_MS=")); _reportStream->println(millis());
}

void Diagnostics::printFooter() const {
  if (_reportStream == nullptr) return;
  printSummary();
  _reportStream->println(F("DIAG,END"));
}

Diagnostics::Result Diagnostics::checkConfiguration(char* d, size_t n) const {
  if (RobotConfig::DRIVE_MOTOR_COUNT != Pins::PHYSICAL_DRIVE_BTS_COUNT) {
    snprintf(d, n, "DRIVE_COUNT_MISMATCH,CFG=%u,PHYSICAL=%u",
             RobotConfig::DRIVE_MOTOR_COUNT, Pins::PHYSICAL_DRIVE_BTS_COUNT);
    return Result::FAIL;
  }
  if (RobotConfig::SLOT_COUNT != SLOT_COUNT) {
    snprintf(d, n, "SLOT_COUNT_MISMATCH,CFG=%u,CONST=%u",
             RobotConfig::SLOT_COUNT, SLOT_COUNT);
    return Result::FAIL;
  }
  snprintf(d, n, "OK,DRIVE_MOTORS=%u,LOGICAL_CHANNELS=%u,SLOTS=%u",
           RobotConfig::DRIVE_MOTOR_COUNT, Pins::BTS_COUNT, RobotConfig::SLOT_COUNT);
  return Result::PASS;
}

bool Diagnostics::pinUsed(int8_t pin) { return pin != Pins::UNUSED_PIN; }

Diagnostics::Result Diagnostics::checkPinAssignments(char* d, size_t n) const {
  int8_t pins[32];
  const char* names[32];
  uint8_t count = 0;
  #define ADD_PIN(P, N) do { if (pinUsed(P) && count < 32) { pins[count] = (P); names[count] = (N); ++count; } } while (0)
  ADD_PIN(Pins::OLED_SDA, "OLED_SDA"); ADD_PIN(Pins::OLED_SCL, "OLED_SCL");
  ADD_PIN(Pins::RFID_RX, "RFID_RX"); ADD_PIN(Pins::RFID_TX, "RFID_TX");
  for (uint8_t i = 0; i < Pins::SERVO_COUNT; ++i) ADD_PIN(Pins::SERVO_PINS[i].pin, Pins::SERVO_PINS[i].name);
  for (uint8_t i = 0; i < Pins::BTS_COUNT; ++i) {
    ADD_PIN(Pins::BTS_PINS[i].rpwm, Pins::BTS_PINS[i].name);
    ADD_PIN(Pins::BTS_PINS[i].lpwm, Pins::BTS_PINS[i].name);
  }
  if (RobotConfig::USE_LINEAR_ACTUATOR) {
    ADD_PIN(Pins::LINEAR_ACTUATOR_PINS.rpwm, "ACT_RPWM");
    ADD_PIN(Pins::LINEAR_ACTUATOR_PINS.lpwm, "ACT_LPWM");
  }
  if (RobotConfig::USE_POWER_MONITORING) ADD_PIN(Pins::BATTERY_ADC, "BATTERY_ADC");
  #undef ADD_PIN

  for (uint8_t i = 0; i < count; ++i) {
    for (uint8_t j = i + 1; j < count; ++j) {
      if (pins[i] == pins[j]) {
        // SDA/D10 ve SCL/D11 gibi bilincli isim eslemeleri listede tek kez bulunur.
        snprintf(d, n, "PIN_CONFLICT,PIN=%d,%s,%s", pins[i], names[i], names[j]);
        return Result::FAIL;
      }
    }
  }
  if (RobotConfig::USE_POWER_MONITORING && Pins::BATTERY_ADC == Pins::UNUSED_PIN) {
    snprintf(d, n, "POWER_ENABLED_BUT_ADC_UNASSIGNED");
    return Result::WARNING;
  }
  snprintf(d, n, "OK,ASSIGNED_SIGNALS=%u,SPARE_BTS=%s", count,
           Pins::SPARE_BTS8_PINS.rpwm == Pins::UNUSED_PIN ? "UNASSIGNED" : "ASSIGNED");
  return Result::PASS;
}

Diagnostics::Result Diagnostics::checkMotionController(char* d, size_t n) const {
  if (_motionController == nullptr || !_motionController->ready()) {
    snprintf(d, n, "NOT_READY"); return Result::FAIL;
  }
  snprintf(d, n, "READY,ESTOP=%u,TARGET_L=%d,TARGET_R=%d,CURRENT_L=%d,CURRENT_R=%d,MAX_PWM=%u",
           _motionController->emergencyStopped(), _motionController->targetLeftPwm(),
           _motionController->targetRightPwm(), _motionController->currentLeftPwm(),
           _motionController->currentRightPwm(), _motionController->maxPwm());
  return _motionController->emergencyStopped() ? Result::WARNING : Result::PASS;
}

Diagnostics::Result Diagnostics::checkMotorDriver(char* d, size_t n) const {
  if (_motorDriver == nullptr || !_motorDriver->ready()) { snprintf(d, n, "NOT_READY"); return Result::FAIL; }
  if (_motorDriver->count() != Pins::BTS_COUNT) {
    snprintf(d, n, "CHANNEL_COUNT_MISMATCH,ACTIVE=%u,EXPECTED=%u", _motorDriver->count(), Pins::BTS_COUNT);
    return Result::FAIL;
  }
  uint8_t moving = 0;
  for (uint8_t i = 0; i < _motorDriver->count(); ++i) {
    const Bts7960Channel* c = _motorDriver->channel(i);
    if (c == nullptr || !c->ready()) { snprintf(d, n, "CHANNEL_%u_NOT_READY", i); return Result::FAIL; }
    if (c->speed() != 0) ++moving;
  }
  snprintf(d, n, "READY,CHANNELS=%u,MOVING=%u", _motorDriver->count(), moving);
  return moving > 0 ? Result::WARNING : Result::PASS;
}

Diagnostics::Result Diagnostics::checkLinearActuator(char* d, size_t n) const {
  if (!RobotConfig::USE_LINEAR_ACTUATOR) { snprintf(d, n, "DISABLED"); return Result::PASS; }
  if (_linearActuator == nullptr || !_linearActuator->ready()) { snprintf(d, n, "NOT_AVAILABLE"); return Result::WARNING; }
  const uint8_t dir = static_cast<uint8_t>(_linearActuator->direction());
  snprintf(d, n, "READY,DIRECTION=%s,REMAINING_MS=%lu,POSITION_FEEDBACK=NONE,LIMIT_SWITCH=NONE",
           actuatorDirectionName(dir), static_cast<unsigned long>(_linearActuator->remainingMs()));
  return _linearActuator->direction() == LinearActuator::Direction::Stop ? Result::WARNING : Result::WARNING;
}

Diagnostics::Result Diagnostics::checkRfid(char* d, size_t n) const {
  if (!RobotConfig::USE_RFID) { snprintf(d, n, "DISABLED"); return Result::PASS; }
  if (_rfid == nullptr || !_rfid->ready()) { snprintf(d, n, "NOT_AVAILABLE"); return Result::WARNING; }
  const uint32_t fw = _rfid->firmwareVersion();
  snprintf(d, n, "READY,BAUD=%lu,FW=0x%08lX,LAST=%s,MAX_UID=%u,TIMEOUT_MS=%u",
           static_cast<unsigned long>(Pins::RFID_BAUD), static_cast<unsigned long>(fw),
           RFID::statusName(_rfid->lastReadStatus()), RFID::MAX_UID_LENGTH, RFID::MAX_READ_TIMEOUT_MS);
  return fw == 0 ? Result::WARNING : Result::PASS;
}

Diagnostics::Result Diagnostics::checkOled(char* d, size_t n) const {
  if (!RobotConfig::USE_OLED) { snprintf(d, n, "DISABLED"); return Result::PASS; }
  if (_oled == nullptr || !_oled->ready()) { snprintf(d, n, "NOT_AVAILABLE,ADDR=0x%02X", Pins::OLED_I2C_ADDRESS); return Result::WARNING; }
  snprintf(d, n, "READY,I2C_ADDR=0x%02X,SIZE=%ux%u", Pins::OLED_I2C_ADDRESS, Pins::OLED_WIDTH, Pins::OLED_HEIGHT);
  return Result::PASS;
}

Diagnostics::Result Diagnostics::checkPower(char* d, size_t n) const {
  if (!RobotConfig::USE_POWER_MONITORING) { snprintf(d, n, "DISABLED"); return Result::PASS; }
  if (Pins::BATTERY_ADC == Pins::UNUSED_PIN) { snprintf(d, n, "ADC_PIN_UNASSIGNED,MONITORING_CANNOT_WORK"); return Result::FAIL; }
  if (_powerManager == nullptr || !_powerManager->ready()) { snprintf(d, n, "NOT_AVAILABLE"); return Result::WARNING; }
  const float adcV = _powerManager->readAdcVoltage();
  const float batteryV = _powerManager->readBatteryVoltage();
  if (isnan(adcV) || isnan(batteryV) || batteryV < 0.1f || batteryV > 15.0f) {
    snprintf(d, n, "INVALID,ADC=%.3fV,BAT=%.2fV", adcV, batteryV); return Result::FAIL;
  }
  snprintf(d, n, "ADC=%.3fV,BAT=%.2fV,PERCENT=%u,LOW_LIMIT=%.2fV",
           adcV, batteryV, _powerManager->estimate3sPercent(), LOW_BATTERY_VOLTAGE);
  return batteryV <= LOW_BATTERY_VOLTAGE ? Result::WARNING : Result::PASS;
}

Diagnostics::Result Diagnostics::checkServoSystem(char* d, size_t n) const {
  if (!RobotConfig::USE_SERVO_SYSTEM) { snprintf(d, n, "DISABLED"); return Result::PASS; }
  if (_servoDriver == nullptr || _servoManager == nullptr || !_servoDriver->ready() || !_servoManager->ready()) {
    snprintf(d, n, "NOT_AVAILABLE"); return Result::WARNING;
  }
  if (_servoDriver->count() != ServoManager::JOINT_COUNT) {
    snprintf(d, n, "COUNT_MISMATCH,DRIVER=%u,EXPECTED=%u", _servoDriver->count(), ServoManager::JOINT_COUNT);
    return Result::FAIL;
  }
  uint8_t moving = 0;
  for (uint8_t i = 0; i < _servoDriver->count(); ++i) if (_servoDriver->angle(i) != _servoDriver->target(i)) ++moving;
  snprintf(d, n, "READY,SERVOS=%u,MOVING=%u,POSITION_FEEDBACK=COMMAND_ESTIMATE_ONLY", _servoDriver->count(), moving);
  return moving > 0 ? Result::WARNING : Result::PASS;
}

Diagnostics::Result Diagnostics::checkJoystick(char* d, size_t n) const {
  if (!RobotConfig::USE_JOYSTICK) { snprintf(d, n, "DISABLED"); return Result::PASS; }
  if (_joystick == nullptr) { snprintf(d, n, "NOT_AVAILABLE"); return Result::FAIL; }
  const JoystickPacket& p = _joystick->packet();
  if (!p.valid) { snprintf(d, n, "NO_VALID_PACKET,TIMEOUT_LIMIT_MS=%u", Pins::JOYSTICK_TIMEOUT_MS); return Result::WARNING; }
  snprintf(d, n, "VALID,TIMED_OUT=%u,X=%d,Y=%d,TWIST=%d,THROTTLE=%u,HAT=%d,BUTTONS=0x%08lX",
           _joystick->timedOut(), p.xPercent, p.yPercent, p.twistPercent, p.throttlePercent,
           p.hatAngle, static_cast<unsigned long>(p.buttons));
  return _joystick->timedOut() ? Result::WARNING : Result::PASS;
}

Diagnostics::Result Diagnostics::checkErrorManager(char* d, size_t n) const {
  if (_errorManager == nullptr) { snprintf(d, n, "NOT_AVAILABLE"); return Result::FAIL; }
  snprintf(d, n, "ACTIVE=%u,CODE=%u", _errorManager->hasError(), static_cast<unsigned>(_errorManager->getCurrentError()));
  return _errorManager->hasError() ? Result::WARNING : Result::PASS;
}

Diagnostics::Result Diagnostics::checkMemory(char* d, size_t n) const {
#if defined(ESP32)
  const uint32_t freeHeap = ESP.getFreeHeap();
  const uint32_t minHeap = ESP.getMinFreeHeap();
  snprintf(d, n, "FREE_HEAP=%lu,MIN_FREE_HEAP=%lu", static_cast<unsigned long>(freeHeap), static_cast<unsigned long>(minHeap));
  if (freeHeap < 10000) return Result::FAIL;
  if (freeHeap < 25000) return Result::WARNING;
  return Result::PASS;
#else
  snprintf(d, n, "UNSUPPORTED_PLATFORM");
  return Result::WARNING;
#endif
}

Diagnostics::Result Diagnostics::checkTiming(char* d, size_t n) const {
  if (CONTROL_LOOP_INTERVAL_MS == 0 || DISPLAY_UPDATE_INTERVAL_MS < CONTROL_LOOP_INTERVAL_MS) {
    snprintf(d, n, "INVALID,CONTROL_MS=%u,DISPLAY_MS=%u", CONTROL_LOOP_INTERVAL_MS, DISPLAY_UPDATE_INTERVAL_MS);
    return Result::FAIL;
  }
  if (Pins::JOYSTICK_TIMEOUT_MS <= CONTROL_LOOP_INTERVAL_MS) {
    snprintf(d, n, "JOYSTICK_TIMEOUT_TOO_SHORT,TIMEOUT=%u,LOOP=%u", Pins::JOYSTICK_TIMEOUT_MS, CONTROL_LOOP_INTERVAL_MS);
    return Result::WARNING;
  }
  snprintf(d, n, "CONTROL_MS=%u,DISPLAY_MS=%u,JOYSTICK_TIMEOUT_MS=%u,SW_WDT_MS=%u,HW_WDT_MS=%lu",
           CONTROL_LOOP_INTERVAL_MS, DISPLAY_UPDATE_INTERVAL_MS, Pins::JOYSTICK_TIMEOUT_MS,
           SOFTWARE_WATCHDOG_TIMEOUT_MS, static_cast<unsigned long>(HARDWARE_WATCHDOG_TIMEOUT_MS));
  return Result::PASS;
}

Diagnostics::Summary Diagnostics::run(bool verbose) {
  ++_runCounter;
  resetSummary();
  const uint32_t startedAt = millis();
  if (!_ready) { record("SERVICE", Result::FAIL, "NOT_INITIALIZED", verbose); return _lastSummary; }
  if (verbose) printHeader();

  char detail[180];
  record("CONFIG", checkConfiguration(detail, sizeof(detail)), detail, verbose);
  record("PINS", checkPinAssignments(detail, sizeof(detail)), detail, verbose);
  record("TIMING", checkTiming(detail, sizeof(detail)), detail, verbose);
  record("MEMORY", checkMemory(detail, sizeof(detail)), detail, verbose);
  record("MOTION", checkMotionController(detail, sizeof(detail)), detail, verbose);
  record("MOTORS", checkMotorDriver(detail, sizeof(detail)), detail, verbose);
  record("ACTUATOR", checkLinearActuator(detail, sizeof(detail)), detail, verbose);
  record("RFID", checkRfid(detail, sizeof(detail)), detail, verbose);
  record("OLED", checkOled(detail, sizeof(detail)), detail, verbose);
  record("POWER", checkPower(detail, sizeof(detail)), detail, verbose);
  record("SERVOS", checkServoSystem(detail, sizeof(detail)), detail, verbose);
  record("JOYSTICK", checkJoystick(detail, sizeof(detail)), detail, verbose);
  record("ERROR_MANAGER", checkErrorManager(detail, sizeof(detail)), detail, verbose);

  _lastSummary.durationMs = millis() - startedAt;
  _lastSummary.completedAtMs = millis();
  if (verbose) { printModuleReport(); printFooter(); }
  return _lastSummary;
}

void Diagnostics::printSummary() const {
  if (_reportStream == nullptr) return;
  _reportStream->print(F("DIAG,SUMMARY,RUN=")); _reportStream->print(_lastSummary.runNumber);
  _reportStream->print(F(",TOTAL=")); _reportStream->print(_lastSummary.total);
  _reportStream->print(F(",PASS=")); _reportStream->print(_lastSummary.passed);
  _reportStream->print(F(",WARNING=")); _reportStream->print(_lastSummary.warnings);
  _reportStream->print(F(",FAIL=")); _reportStream->print(_lastSummary.failed);
  _reportStream->print(F(",HEALTHY=")); _reportStream->print(_lastSummary.healthy() ? 1 : 0);
  _reportStream->print(F(",DURATION_MS=")); _reportStream->print(_lastSummary.durationMs);
  _reportStream->print(F(",COMPLETED_AT_MS=")); _reportStream->println(_lastSummary.completedAtMs);
}

void Diagnostics::printLiveStatus() const {
  if (_reportStream == nullptr || !_ready) return;
  _reportStream->print(F("DIAG,LIVE,UPTIME_MS=")); _reportStream->print(millis());
  if (_motionController) {
    _reportStream->print(F(",ESTOP=")); _reportStream->print(_motionController->emergencyStopped());
    _reportStream->print(F(",TARGET_L=")); _reportStream->print(_motionController->targetLeftPwm());
    _reportStream->print(F(",TARGET_R=")); _reportStream->print(_motionController->targetRightPwm());
    _reportStream->print(F(",CURRENT_L=")); _reportStream->print(_motionController->currentLeftPwm());
    _reportStream->print(F(",CURRENT_R=")); _reportStream->print(_motionController->currentRightPwm());
  }
  if (_joystick) { _reportStream->print(F(",JOY_VALID=")); _reportStream->print(_joystick->packet().valid); _reportStream->print(F(",JOY_TIMEOUT=")); _reportStream->print(_joystick->timedOut()); }
  if (_errorManager) { _reportStream->print(F(",ERROR=")); _reportStream->print(static_cast<unsigned>(_errorManager->getCurrentError())); }
  _reportStream->println();
}

void Diagnostics::printPinReport() const {
  if (_reportStream == nullptr) return;
  _reportStream->print(F("DIAG,PIN,OLED,SDA=")); _reportStream->print(Pins::OLED_SDA); _reportStream->print(F(",SCL=")); _reportStream->println(Pins::OLED_SCL);
  _reportStream->print(F("DIAG,PIN,RFID,RX=")); _reportStream->print(Pins::RFID_RX); _reportStream->print(F(",TX=")); _reportStream->println(Pins::RFID_TX);
  for (uint8_t i = 0; i < Pins::BTS_COUNT; ++i) {
    _reportStream->print(F("DIAG,PIN,MOTOR,")); _reportStream->print(i); _reportStream->print(','); _reportStream->print(Pins::BTS_PINS[i].name);
    _reportStream->print(F(",RPWM=")); _reportStream->print(Pins::BTS_PINS[i].rpwm); _reportStream->print(F(",LPWM=")); _reportStream->println(Pins::BTS_PINS[i].lpwm);
  }
  _reportStream->print(F("DIAG,PIN,ACTUATOR,RPWM=")); _reportStream->print(Pins::LINEAR_ACTUATOR_PINS.rpwm); _reportStream->print(F(",LPWM=")); _reportStream->println(Pins::LINEAR_ACTUATOR_PINS.lpwm);
  for (uint8_t i = 0; i < Pins::SERVO_COUNT; ++i) {
    _reportStream->print(F("DIAG,PIN,SERVO,")); _reportStream->print(i); _reportStream->print(','); _reportStream->print(Pins::SERVO_PINS[i].name);
    _reportStream->print(F(",PIN=")); _reportStream->print(Pins::SERVO_PINS[i].pin); _reportStream->print(F(",MIN=")); _reportStream->print(Pins::SERVO_PINS[i].minAngle);
    _reportStream->print(F(",MAX=")); _reportStream->print(Pins::SERVO_PINS[i].maxAngle); _reportStream->print(F(",START=")); _reportStream->println(Pins::SERVO_PINS[i].startAngle);
  }
  _reportStream->print(F("DIAG,PIN,POWER,ADC=")); _reportStream->println(Pins::BATTERY_ADC);
}

void Diagnostics::printModuleReport() const {
  if (_reportStream == nullptr) return;
  if (_motorDriver) {
    for (uint8_t i = 0; i < _motorDriver->count(); ++i) {
      const Bts7960Channel* c = _motorDriver->channel(i);
      if (!c) continue;
      _reportStream->print(F("DIAG,MODULE,MOTOR,")); _reportStream->print(i); _reportStream->print(F(",NAME=")); _reportStream->print(c->name());
      _reportStream->print(F(",READY=")); _reportStream->print(c->ready()); _reportStream->print(F(",SPEED=")); _reportStream->print(c->speed());
      _reportStream->print(F(",STATE=")); _reportStream->println(motorStateName(static_cast<uint8_t>(c->state())));
    }
  }
  if (_servoDriver && _servoDriver->ready()) {
    for (uint8_t i = 0; i < _servoDriver->count(); ++i) {
      _reportStream->print(F("DIAG,MODULE,SERVO,")); _reportStream->print(i); _reportStream->print(F(",ANGLE=")); _reportStream->print(_servoDriver->angle(i));
      _reportStream->print(F(",TARGET=")); _reportStream->println(_servoDriver->target(i));
    }
  }
}

const Diagnostics::Summary& Diagnostics::lastSummary() const { return _lastSummary; }
bool Diagnostics::ready() const { return _ready; }

const char* Diagnostics::resultName(Result r) {
  switch (r) { case Result::PASS: return "PASS"; case Result::WARNING: return "WARNING"; case Result::FAIL: return "FAIL"; }
  return "UNKNOWN";
}

const char* Diagnostics::motorStateName(uint8_t s) {
  switch (static_cast<Bts7960Channel::State>(s)) {
    case Bts7960Channel::State::NotReady: return "NOT_READY";
    case Bts7960Channel::State::Coast: return "COAST";
    case Bts7960Channel::State::Forward: return "FORWARD";
    case Bts7960Channel::State::Reverse: return "REVERSE";
  }
  return "UNKNOWN";
}

const char* Diagnostics::actuatorDirectionName(uint8_t d) {
  switch (static_cast<LinearActuator::Direction>(d)) {
    case LinearActuator::Direction::Stop: return "STOP";
    case LinearActuator::Direction::Extend: return "EXTEND";
    case LinearActuator::Direction::Retract: return "RETRACT";
  }
  return "UNKNOWN";
}
