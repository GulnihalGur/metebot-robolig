#pragma once

#include <Arduino.h>
#include <Pins.h>
#include <Types.h>

// Joystick verilerini ayristirir ve suruse cevirir.
class Joystick {
public:
  Joystick();

  // format:
  // JOY,x,y,twist,throttle,hat,buttons

  bool parseLine(const String& line);

  // Sol ve sag motor PWM degerlerini hesaplar.
  DifferentialOutput differential(uint8_t maxPwm = 200) const;

  // Gaz koluna gore maksimum PWM hesaplar.
  uint8_t throttlePwm(uint8_t minPwm = 70, uint8_t maxPwm = 255) const;

  // Belirtilen buton basili mi kontrol eder.
  bool buttonPressed(uint8_t buttonIndex) const;

  // Butona bu pakette yeni basildi mi kontrol eder.
  bool buttonJustPressed(uint8_t buttonIndex) const;

  // Veri zaman asimina ugradi mi kontrol eder.
  bool timedOut(uint16_t timeoutMs = Pins::JOYSTICK_TIMEOUT_MS) const;

  // Son joystick paketini dondurur.
  const JoystickPacket& packet() const;

private:
  // Son alinan joystick paketi.
  JoystickPacket _packet;

  // Bir onceki paketin buton bit maskesi.
  uint32_t _previousButtons;

  // Komut baslangicini kontrol eder.
  static bool startsWithCommand(const String& line, const char* command);

  // CSV satirini alanlara ayirir.
  static uint8_t splitCsv(const String& line, String* fields, uint8_t maxFields);

  // Eksen degerini yuzdeye cevirir.
  static int16_t normalizeAxis(long value);

  // Gaz kolunu yuzdeye cevirir.
  static uint8_t normalizeThrottle(long value);

  // Merkezdeki titresimi yok sayar.
  static int16_t applyDeadzone(int16_t value, uint8_t deadzonePercent);
};
