#pragma once

#include <Arduino.h>
#include <math.h>
#include <Pins.h>

// Batarya voltajini ve yuzdesini takip eden sinif.
class PowerManager {
public:
  PowerManager();

  // ADC pini ve voltaj bolucu degerlerini ayarlar.
  bool begin(int adcPin = Pins::BATTERY_ADC,
             float rTopOhm = 30000.0f,
             float rBottomOhm = 7500.0f,
             float adcReferenceVoltage = 3.3f,
             uint16_t adcMax = 4095);

  // ADC pinindeki voltaji dondurur.
  float readAdcVoltage() const;

  // Gercek batarya voltajini dondurur.
  float readBatteryVoltage() const;

  // 3S Li-Po yuzdesini tahmin eder.
  uint8_t estimate3sPercent() const;

  // Dusuk batarya durumunu kontrol eder.
  bool lowBattery(float thresholdVoltage = 10.5f) const;

  // Sinif hazir mi kontrol eder.
  bool ready() const;

private:
  // Batarya olcum ADC pini.
  int _adcPin;

  // Voltaj bolucunun ust direnci.
  float _rTopOhm;

  // Voltaj bolucunun alt direnci.
  float _rBottomOhm;

  // ADC referans voltaji.
  float _adcReferenceVoltage;

  // ADC'nin en buyuk sayisal degeri.
  uint16_t _adcMax;

  // Baslatma durumu.
  bool _ready;
};
