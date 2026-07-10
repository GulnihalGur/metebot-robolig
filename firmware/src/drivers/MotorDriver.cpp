#include "MotorDriver.h"

// Tek bir BTS7960 kanalinin varsayilan degerlerini ayarlar.
Bts7960Channel::Bts7960Channel()
  : _name("unassigned"),
    _rpwmPin(Pins::UNUSED_PIN),
    _lpwmPin(Pins::UNUSED_PIN),
    _invertDirection(false),
    _ready(false),
    _speed(0),
    _state(State::NotReady) {}

// Pin tablosundaki BTS bilgileriyle kanali baslatir.
bool Bts7960Channel::begin(const Pins::BtsPins& pins) {
  return begin(pins.name, pins.rpwm, pins.lpwm, pins.invertDirection);
}

// BTS7960 pinlerini ve yon ayarini kaydeder.
bool Bts7960Channel::begin(const char* name, int8_t rpwmPin, int8_t lpwmPin, bool invertDirection) {
  _name = name;
  _rpwmPin = rpwmPin;
  _lpwmPin = lpwmPin;
  _invertDirection = invertDirection;

  // Pinler gecersizse kanal hazir olmaz.
  if (_rpwmPin < 0 || _lpwmPin < 0) {
    _ready = false;
    _speed = 0;
    _state = State::NotReady;
    return false;
  }

  // PWM pinlerini cikis olarak ayarlar.
  pinMode(_rpwmPin, OUTPUT);
  pinMode(_lpwmPin, OUTPUT);

  _ready = true;
  stop();
  return true;
}

// Motor hizini ve yonunu ayarlar.
void Bts7960Channel::setSpeed(int16_t speedValue) {
  if (!_ready) return;

  // Gerekiyorsa motor yonunu ters cevirir.
  if (_invertDirection) speedValue = -speedValue;

  // Hizi izin verilen PWM araliginda tutar.
  speedValue = constrain(speedValue, -Pins::MOTOR_PWM_MAX, Pins::MOTOR_PWM_MAX);
  _speed = speedValue;

  // Pozitif deger ileri hareket verir.
  if (speedValue > 0) {
    _state = State::Forward;
    writePwm(clampPwm(speedValue), 0);

  // Negatif deger geri hareket verir.
  } else if (speedValue < 0) {
    _state = State::Reverse;
    writePwm(0, clampPwm(-speedValue));

  // Sifir deger motoru durdurur.
  } else {
    stop();
  }
}

// Motoru bosta durdurur.
void Bts7960Channel::stop() {
  if (!_ready) return;

  _speed = 0;
  _state = State::Coast;

  // Fail-safe icin iki PWM de sifirlanir.
  writePwm(0, 0);
}

// Kullaniciya gore motor hizini dondurur.
int16_t Bts7960Channel::speed() const {
  return _invertDirection ? -_speed : _speed;
}

// Motorun mevcut durumunu dondurur.
Bts7960Channel::State Bts7960Channel::state() const {
  return _state;
}

// Kanal kullanima hazir mi kontrol eder.
bool Bts7960Channel::ready() const {
  return _ready;
}

// Kanal adini dondurur.
const char* Bts7960Channel::name() const {
  return _name;
}

// PWM degerini gecerli aralikta tutar.
uint8_t Bts7960Channel::clampPwm(int16_t value) {
  return static_cast<uint8_t>(constrain(value, 0, Pins::MOTOR_PWM_MAX));
}

// RPWM ve LPWM pinlerine deger yazar.
void Bts7960Channel::writePwm(uint8_t rpwmValue, uint8_t lpwmValue) {
  analogWrite(_rpwmPin, rpwmValue);
  analogWrite(_lpwmPin, lpwmValue);
}

// MotorDriver varsayilan degerlerini ayarlar.
MotorDriver::MotorDriver()
  : _count(0), _ready(false) {}

// Tum BTS7960 kanallarini baslatir.
bool MotorDriver::begin(const Pins::BtsPins* pins, uint8_t countValue) {
  if (pins == nullptr) return false;

  // Kanal sayisini maksimum degerle sinirlar.
  _count = min<uint8_t>(countValue, MAX_CHANNELS);
  bool allReady = true;

  // Her motor kanalini ayri ayri baslatir.
  for (uint8_t i = 0; i < _count; i++) {
    allReady = _channels[i].begin(pins[i]) && allReady;
  }

  // Baslangicta tum motorlari durdurur.
  stopAll();

  _ready = allReady;
  return _ready;
}

// Belirtilen motor kanalinin hizini ayarlar.
bool MotorDriver::setChannel(uint8_t index, int16_t speedValue) {
  if (index >= _count) return false;
  if (!_channels[index].ready()) return false;

  _channels[index].setSpeed(speedValue);
  return true;
}

// Belirtilen motor kanalini durdurur.
bool MotorDriver::stopChannel(uint8_t index) {
  if (index >= _count) return false;
  if (!_channels[index].ready()) return false;

  _channels[index].stop();
  return true;
}

// Bir motor grubuna ayni hizi uygular.
bool MotorDriver::setGroup(const uint8_t* indices, uint8_t size, int16_t speed) {
  bool ok = true;

  for (uint8_t i = 0; i < size; i++) {
    ok = setChannel(indices[i], speed) && ok;
  }

  return ok;
}

// Bir motor grubunu durdurur.
void MotorDriver::stopGroup(const uint8_t* indices, uint8_t size) {
  for (uint8_t i = 0; i < size; i++) {
    stopChannel(indices[i]);
  }
}

// Sol taraftaki uc motorun hizini ayarlar.
bool MotorDriver::setLeftSide(int16_t leftSpeed) {
  return setGroup(Pins::DRIVE_LEFT_INDICES, 3, leftSpeed);
}

// Sag taraftaki uc motorun hizini ayarlar.
bool MotorDriver::setRightSide(int16_t rightSpeed) {
  return setGroup(Pins::DRIVE_RIGHT_INDICES, 3, rightSpeed);
}

// Sol ve sag motor hizlarini birlikte ayarlar.
bool MotorDriver::setDriveSpeeds(int16_t leftSpeed, int16_t rightSpeed) {
  bool leftOk = setLeftSide(leftSpeed);
  bool rightOk = setRightSide(rightSpeed);

  return leftOk && rightOk;
}

// Alti surus motorunu durdurur.
void MotorDriver::stopDrive() {
  stopGroup(Pins::DRIVE_LEFT_INDICES, 3);
  stopGroup(Pins::DRIVE_RIGHT_INDICES, 3);
}

// Tum motor kanallarini durdurur.
void MotorDriver::stopAll() {
  for (uint8_t i = 0; i < _count; i++) {
    _channels[i].stop();
  }
}

// Belirtilen kanal nesnesinin adresini dondurur.
Bts7960Channel* MotorDriver::channel(uint8_t index) {
  if (index >= _count) return nullptr;
  return &_channels[index];
}

// Sabit kullanim icin kanal adresini dondurur.
const Bts7960Channel* MotorDriver::channel(uint8_t index) const {
  if (index >= _count) return nullptr;
  return &_channels[index];
}

// Aktif kanal sayisini dondurur.
uint8_t MotorDriver::count() const {
  return _count;
}

// Tum motor suruculeri hazir mi kontrol eder.
bool MotorDriver::ready() const {
  return _ready;
}
