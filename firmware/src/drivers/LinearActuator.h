#pragma once

#include <Arduino.h>
#include <Pins.h>
#include <drivers/MotorDriver/MotorDriver.h>

// Lineer aktuatoru BTS7960 ile kontrol eder.
class LinearActuator {
public:
  // Aktuatorun olasi hareket yonleri.
  enum class Direction : uint8_t {
    Stop,
    Extend,
    Retract
  };

  LinearActuator();

  // BTS7960 kanalini ve pinlerini baslatir.
  bool begin(const Pins::BtsPins& pins = Pins::LINEAR_ACTUATOR_PINS);

  // Aktuatori disari uzatir.
  void extend(uint8_t speed = 255);

  // Aktuatori iceri ceker.
  void retract(uint8_t speed = 255);

  // Aktuator hareketini durdurur.
  void stop();

  // Belirli sure boyunca hareket ettirir.
  void moveFor(Direction direction, uint32_t durationMs, uint8_t speed = 200);

  // Sure dolunca otomatik durdurur.
  void update();

  // Aktuator hazir mi kontrol eder.
  bool ready() const;

  // Mevcut hareket yonunu dondurur.
  Direction direction() const;

  // Durdurmaya kalan sureyi dondurur.
  uint32_t remainingMs() const;

private:
  // Aktuatoru suren BTS7960 kanali.
  Bts7960Channel _channel;

  // Baslatma durumu.
  bool _ready;

  // Mevcut hareket yonu.
  Direction _direction;

  // Otomatik durma zamani.
  uint32_t _stopAtMs;
};
