#pragma once

#include <Arduino.h>
#include <Pins.h>

// Joystickten gelen verileri tutar.
struct JoystickPacket {
  int16_t xPercent;          // Sag sol ekseni
  int16_t yPercent;          // Ileri geri ekseni
  int16_t twistPercent;      // Sap cevirme ekseni
  uint8_t throttlePercent;   // Gaz kolu yuzdesi
  int16_t hatAngle;          // Hat switch acisi
  uint32_t buttons;          // Buton bit maskesi
  uint32_t timestampMs;      // Son paket zamani
  bool valid;                // Paket gecerli mi
};

// Sol ve sag motor PWM degerlerini tutar.
struct DifferentialOutput {
  int16_t leftPwm;
  int16_t rightPwm;
};

// Joystick verilerini ayristirir ve suruse cevirir.
class Joystick {
public:
  Joystick();

  // Yeni format:
  // JOY,x,y,twist,throttle,hat,buttons
  //
  // Eski format:
  // JOY,x,y,buttons
  bool parseLine(const String& line);

  // Sol ve sag motor PWM degerlerini hesaplar.
  DifferentialOutput differential(uint8_t maxPwm = 200) const;

  // Gaz koluna gore maksimum PWM hesaplar.
  uint8_t throttlePwm(uint8_t minPwm = 70, uint8_t maxPwm = 255) const;

  // Belirtilen buton basili mi kontrol eder.
  bool buttonPressed(uint8_t buttonIndex) const;

  // Veri zaman asimina ugradi mi kontrol eder.
  bool timedOut(uint16_t timeoutMs = Pins::JOYSTICK_TIMEOUT_MS) const;

  // Son joystick paketini dondurur.
  const JoystickPacket& packet() const;

private:
  // Son alinan joystick paketi.
  JoystickPacket _packet;

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