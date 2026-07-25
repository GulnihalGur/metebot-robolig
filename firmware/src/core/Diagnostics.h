pragma once

#include <Arduino.h>

class MotionController;
class MotorDriver;
class LinearActuator;
class RFID;
class OLED;
class PowerManager;
class ServoDriver;
class ServoManager;
class Joystick;
class ErrorManager;

// Robot modullerini hareket olusturmadan kontrol eden merkezi tani servisi.
class Diagnostics {
public:
  enum class Result : uint8_t { PASS, WARNING, FAIL };

  struct Summary {
    uint8_t passed;
    uint8_t warnings;
    uint8_t failed;
    uint8_t total;
    uint32_t runNumber;
    uint32_t durationMs;
    uint32_t completedAtMs;

    bool healthy() const { return failed == 0; }
  };

  Diagnostics();

  bool begin(MotionController& motionController,
             MotorDriver& motorDriver,
             LinearActuator* linearActuator,
             RFID* rfid,
             OLED* oled,
             PowerManager* powerManager,
             ServoDriver* servoDriver,
             ServoManager* servoManager,
             Joystick* joystick,
             ErrorManager& errorManager,
             Stream& reportStream = Serial);

  // Tum guvenli kontrolleri calistirir. Motor, servo veya aktuatoru hareket ettirmez.
  Summary run(bool verbose = true);

  // Son kosunun ozetini, canli durumu veya pin tablosunu raporlar.
  void printSummary() const;
  void printLiveStatus() const;
  void printPinReport() const;
  void printModuleReport() const;

  const Summary& lastSummary() const;
  bool ready() const;
  static const char* resultName(Result result);

private:
  MotionController* _motionController;
  MotorDriver* _motorDriver;
  LinearActuator* _linearActuator;
  RFID* _rfid;
  OLED* _oled;
  PowerManager* _powerManager;
  ServoDriver* _servoDriver;
  ServoManager* _servoManager;
  Joystick* _joystick;
  ErrorManager* _errorManager;
  Stream* _reportStream;

  Summary _lastSummary;
  bool _ready;
  uint32_t _runCounter;

  void resetSummary();
  void record(const char* testName, Result result, const char* detail, bool verbose);
  void printHeader() const;
  void printFooter() const;

  Result checkConfiguration(char* detail, size_t size) const;
  Result checkPinAssignments(char* detail, size_t size) const;
  Result checkMotionController(char* detail, size_t size) const;
  Result checkMotorDriver(char* detail, size_t size) const;
  Result checkLinearActuator(char* detail, size_t size) const;
  Result checkRfid(char* detail, size_t size) const;
  Result checkOled(char* detail, size_t size) const;
  Result checkPower(char* detail, size_t size) const;
  Result checkServoSystem(char* detail, size_t size) const;
  Result checkJoystick(char* detail, size_t size) const;
  Result checkErrorManager(char* detail, size_t size) const;
  Result checkMemory(char* detail, size_t size) const;
  Result checkTiming(char* detail, size_t size) const;

  static const char* motorStateName(uint8_t state);
  static const char* actuatorDirectionName(uint8_t direction);
  static bool pinUsed(int8_t pin);
};
