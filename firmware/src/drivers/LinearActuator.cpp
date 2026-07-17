#include "LinearActuator.h"

// Varsayilan durum degerlerini ayarlar.
LinearActuator::LinearActuator()
  : _ready(false), _direction(Direction::Stop), _stopAtMs(0) {}

// BTS7960 kanalini baslatir.
bool LinearActuator::begin(const Pins::BtsPins& pins) {
  _ready = _channel.begin(pins);

  // Baslangicta aktuatoru durdurur.
  stop();
  return _ready;
}

// Aktuatori disari dogru hareket ettirir.
void LinearActuator::extend(uint8_t speed) {
  if (!_ready) return;

  _direction = Direction::Extend;

  // Zamanli durdurmayi iptal eder.
  _stopAtMs = 0;

  // Pozitif hiz uzatma yonudur.
  _channel.setSpeed(speed);
}

// Aktuatori iceri dogru hareket ettirir.
void LinearActuator::retract(uint8_t speed) {
  if (!_ready) return;

  _direction = Direction::Retract;

  // Zamanli durdurmayi iptal eder.
  _stopAtMs = 0;

  // Negatif hiz geri cekme yonudur.
  _channel.setSpeed(-static_cast<int16_t>(speed));
}

// Aktuator hareketini durdurur.
void LinearActuator::stop() {
  if (!_ready) return;

  _direction = Direction::Stop;
  _stopAtMs = 0;
  _channel.stop();
}

// Aktuatori belirli sure boyunca hareket ettirir.
void LinearActuator::moveFor(Direction directionValue, uint32_t durationMs, uint8_t speed) {
  if (!_ready) return;

  // Secilen yone gore hareketi baslatir.
  if (directionValue == Direction::Extend) {
    extend(speed);
  } else if (directionValue == Direction::Retract) {
    retract(speed);
  } else {
    stop();
    return;
  }

  // Otomatik durma zamanini hesaplar.
  _stopAtMs = millis() + durationMs;
}

// Sure doldugunda aktuatori otomatik durdurur.
void LinearActuator::update() {
  // Zamanli hareket yoksa islem yapmaz.
  if (!_ready || _direction == Direction::Stop || _stopAtMs == 0) return;

  // Kalan sureyi hesaplar.
  int32_t delta = static_cast<int32_t>(_stopAtMs - millis());

  // Sure bittiyse hareketi durdurur.
  if (delta <= 0) {
    stop();
  }
}

// Aktuator kullanima hazir mi kontrol eder.
bool LinearActuator::ready() const {
  return _ready;
}

// Mevcut hareket yonunu dondurur.
LinearActuator::Direction LinearActuator::direction() const {
  return _direction;
}

// Otomatik durmaya kalan sureyi dondurur.
uint32_t LinearActuator::remainingMs() const {
  if (_stopAtMs == 0) return 0;

  int32_t delta = static_cast<int32_t>(_stopAtMs - millis());

  return delta > 0 ? static_cast<uint32_t>(delta) : 0;
}
