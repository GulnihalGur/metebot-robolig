#pragma once

#include <Arduino.h>
#include <Pins.h>

// Tek bir BTS7960 motor kanalini yonetir.
class Bts7960Channel {
public:
  // Motor kanalinin olasi durumlari.
  enum class State : uint8_t {
    NotReady,
    Coast,
    Forward,
    Reverse
  };

  Bts7960Channel();

  // Pin tablosundaki bilgilerle kanali baslatir.
  bool begin(const Pins::BtsPins& pins);

  // Pinleri elle vererek kanali baslatir.
  bool begin(const char* name, int8_t rpwmPin, int8_t lpwmPin, bool invertDirection = false);

  // Hiz araligi: -255 ile 255.
  // Pozitif ileri, negatif geri hareket verir.
  void setSpeed(int16_t speed);

  // Motoru durdurur.
  void stop();

  // Mevcut hiz degerini dondurur.
  int16_t speed() const;

  // Mevcut motor durumunu dondurur.
  State state() const;

  // Kanal hazir mi kontrol eder.
  bool ready() const;

  // Kanal adini dondurur.
  const char* name() const;

private:
  // Motor kanalinin adi.
  const char* _name;

  // Ileri yon PWM pini.
  int8_t _rpwmPin;

  // Geri yon PWM pini.
  int8_t _lpwmPin;

  // Motor yonu ters mi tutulacak.
  bool _invertDirection;

  // Kanalin baslatilma durumu.
  bool _ready;

  // Mevcut hiz degeri.
  int16_t _speed;

  // Mevcut motor durumu.
  State _state;

  // PWM degerlerini cikislara yazar.
  void writePwm(uint8_t rpwmValue, uint8_t lpwmValue);

  // PWM degerini gecerli aralikta tutar.
  static uint8_t clampPwm(int16_t value);
};

// Sol ve sag ortak kontrol hatlarini yoneten iki mantiksal BTS7960 kanalini yonetir.
class MotorDriver {
public:
  // Kullanilabilecek en fazla kanal sayisi.
  static constexpr uint8_t MAX_CHANNELS = Pins::BTS_COUNT;

  MotorDriver();

  // Tum motor surucu kanallarini baslatir.
  bool begin(const Pins::BtsPins* pins = Pins::BTS_PINS, uint8_t count = Pins::BTS_COUNT);

  // Tek bir motorun hizini ayarlar.
  bool setChannel(uint8_t index, int16_t speed);

  // Tek bir motoru durdurur.
  bool stopChannel(uint8_t index);

  // Sol taraftaki uc fiziksel BTS surucusunun ortak kontrol hattini surer.
  bool setLeftSide(int16_t leftSpeed);

  // Sag taraftaki uc fiziksel BTS surucusunun ortak kontrol hattini surer.
  bool setRightSide(int16_t rightSpeed);

  // Sol ve sag tarafi farkli hizlarla surer.
  bool setDriveSpeeds(int16_t leftSpeed, int16_t rightSpeed);

  // Sadece surus motorlarini durdurur.
  void stopDrive();

  // Tum motor kanallarini durdurur.
  void stopAll();

  // Belirtilen motor kanalina erisim verir.
  Bts7960Channel* channel(uint8_t index);

  // Sabit kullanim icin kanala erisim verir.
  const Bts7960Channel* channel(uint8_t index) const;

  // Aktif kanal sayisini dondurur.
  uint8_t count() const;

  // Tum kanallar hazir mi kontrol eder.
  bool ready() const;

private:
  // BTS7960 kanal nesnelerini tutar.
  Bts7960Channel _channels[MAX_CHANNELS];

  // Aktif kanal sayisi.
  uint8_t _count;

  // Genel hazirlik durumu.
  bool _ready;

};
