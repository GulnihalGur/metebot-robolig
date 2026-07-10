#include "PowerManager.h"

// Varsayilan batarya olcum degerlerini ayarlar.
PowerManager::PowerManager()
  : _adcPin(Pins::UNUSED_PIN),
    _rTopOhm(30000.0f),
    _rBottomOhm(7500.0f),
    _adcReferenceVoltage(3.3f),
    _adcMax(4095),
    _ready(false) {}

// ADC pini ve voltaj bolucu degerlerini ayarlar.
bool PowerManager::begin(int adcPin, float rTopOhm, float rBottomOhm,
                         float adcReferenceVoltage, uint16_t adcMax) {
  _adcPin = adcPin;
  _rTopOhm = rTopOhm;
  _rBottomOhm = rBottomOhm;
  _adcReferenceVoltage = adcReferenceVoltage;
  _adcMax = adcMax;

  // Gecerli ADC pini yoksa baslatma basarisiz olur.
  if (_adcPin == Pins::UNUSED_PIN) {
    _ready = false;
    return false;
  }

  // ADC pinini giris olarak ayarlar.
  pinMode(_adcPin, INPUT);

  _ready = true;
  return true;
}

// ADC pinindeki voltaji hesaplar.
float PowerManager::readAdcVoltage() const {
  if (!_ready) return NAN;

  // ADC ham degerini okur.
  uint16_t raw = analogRead(_adcPin);

  // Ham ADC degerini volta cevirir.
  return (static_cast<float>(raw) * _adcReferenceVoltage) / static_cast<float>(_adcMax);
}

// Voltaj bolucuden gercek batarya voltajini hesaplar.
float PowerManager::readBatteryVoltage() const {
  if (!_ready) return NAN;

  float adcVoltage = readAdcVoltage();

  // Voltaj bolucu oranini hesaplar.
  float ratio = (_rTopOhm + _rBottomOhm) / _rBottomOhm;

  return adcVoltage * ratio;
}

// 3S Li-Po batarya yuzdesini tahmin eder.
uint8_t PowerManager::estimate3sPercent() const {
  float v = readBatteryVoltage();
  if (isnan(v)) return 0;

  // 9.9V ile 12.6V arasini yuzdeye cevirir.
  float pct = ((v - 9.9f) / (12.6f - 9.9f)) * 100.0f;

  // Sonucu 0 ile 100 arasinda sinirlar.
  return constrain(static_cast<int>(pct), 0, 100);
}

// Batarya voltaji esik degerin altinda mi kontrol eder.
bool PowerManager::lowBattery(float thresholdVoltage) const {
  float v = readBatteryVoltage();
  if (isnan(v)) return false;

  return v <= thresholdVoltage;
}

// Sinif kullanima hazir mi kontrol eder.
bool PowerManager::ready() const {
  return _ready;
}
