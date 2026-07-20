#include "ServoDriver.h"

ServoDriver::ServoDriver()
  : _count(0), _ready(false) {}

bool ServoDriver::begin(const Pins::ServoPin* pins, uint8_t countValue) {
  _ready = false;

  // Yeniden baslatma veya gecersiz ayar durumunda eski PWM cikislarini kapatir.
  detachAll();

  if (pins == nullptr || countValue == 0) {
    _count = 0;
    return false;
  }

  _count = min<uint8_t>(countValue, SERVO_COUNT);

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  // Tum durum alanlarini attach denemesinden once hazirlar.
  // Boylece hata halinde detachAll() guvenle cagrilabilir.
  for (uint8_t i = 0; i < _count; ++i) {
    _servos[i].config = &pins[i];
    _servos[i].currentAngle = limitAngle(pins[i].startAngle, pins[i]);
    _servos[i].targetAngle = _servos[i].currentAngle;
    _servos[i].speedDegPerSec = 120;
    _servos[i].lastUpdateMs = millis();
    _servos[i].attached = false;
  }

  for (uint8_t i = 0; i < _count; ++i) {
    ServoState& state = _servos[i];

    state.servo.setPeriodHertz(50);
    const int channel = state.servo.attach(pins[i].pin, 500, 2500);

    // Kutuphane surumleri kanal 0 konusunda farkli davranabildigi icin
    // basari yalnizca kanal numarasina bakilarak belirlenmez. Negatif kanal
    // veya attached() == false durumu baslatma hatasi kabul edilir.
    if (channel < 0 || !state.servo.attached()) {
      state.attached = false;
      detachAll();
      _ready = false;
      return false;
    }

    state.attached = true;

    // Baslangicta startAngle/home acisina zorunlu hareket komutu gonderilmez.
    // Surucunun attach sonrasi tuttugu aci yazilim durumuna aktarilir.
    const int attachedAngle = constrain(state.servo.read(), 0, 180);
    state.currentAngle = limitAngle(
      static_cast<uint8_t>(attachedAngle),
      pins[i]
    );
    state.targetAngle = state.currentAngle;
    state.lastUpdateMs = millis();
  }

  _ready = true;
  return true;
}

bool ServoDriver::moveTo(uint8_t index, uint8_t targetAngle, uint16_t speedDegPerSec) {
  if (!_ready || index >= _count || !_servos[index].attached) return false;

  const Pins::ServoPin& cfg = *_servos[index].config;
  _servos[index].targetAngle = limitAngle(targetAngle, cfg);
  _servos[index].speedDegPerSec = max<uint16_t>(1, speedDegPerSec);
  return true;
}

bool ServoDriver::writeNow(uint8_t index, uint8_t angleValue) {
  if (!_ready || index >= _count || !_servos[index].attached) return false;

  const Pins::ServoPin& cfg = *_servos[index].config;
  uint8_t safeAngle = limitAngle(angleValue, cfg);
  _servos[index].currentAngle = safeAngle;
  _servos[index].targetAngle = safeAngle;
  _servos[index].servo.write(safeAngle);
  _servos[index].lastUpdateMs = millis();
  return true;
}

bool ServoDriver::detach(uint8_t index) {
  if (index >= _count || !_servos[index].attached) return false;
  _servos[index].servo.detach();
  _servos[index].attached = false;
  return true;
}

void ServoDriver::detachAll() {
  for (uint8_t i = 0; i < _count; ++i) {
    if (_servos[i].attached) {
      _servos[i].servo.detach();
      _servos[i].attached = false;
    }
  }
}

void ServoDriver::update() {
  if (!_ready) return;

  uint32_t now = millis();

  for (uint8_t i = 0; i < _count; i++) {
    ServoState& s = _servos[i];
    if (!s.attached || s.currentAngle == s.targetAngle) continue;

    uint32_t elapsed = now - s.lastUpdateMs;
    if (elapsed < 15) continue;

    uint16_t step = max<uint16_t>(1, (static_cast<uint32_t>(s.speedDegPerSec) * elapsed) / 1000UL);

    if (s.currentAngle < s.targetAngle) {
      s.currentAngle = static_cast<uint8_t>(min<int>(s.targetAngle, static_cast<int>(s.currentAngle) + static_cast<int>(step)));
    } else {
      s.currentAngle = (step >= (s.currentAngle - s.targetAngle))
                         ? s.targetAngle
                         : static_cast<uint8_t>(s.currentAngle - step);
    }

    s.servo.write(s.currentAngle);
    s.lastUpdateMs = now;
  }
}

uint8_t ServoDriver::angle(uint8_t index) const {
  if (index >= _count) return 0;
  return _servos[index].currentAngle;
}

uint8_t ServoDriver::target(uint8_t index) const {
  if (index >= _count) return 0;
  return _servos[index].targetAngle;
}

uint8_t ServoDriver::count() const {
  return _count;
}

bool ServoDriver::ready() const {
  return _ready;
}

uint8_t ServoDriver::limitAngle(uint8_t value, const Pins::ServoPin& cfg) {
  return constrain(value, cfg.minAngle, cfg.maxAngle);
}
