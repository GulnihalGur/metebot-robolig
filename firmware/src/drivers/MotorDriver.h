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

// On tekerleri ayri, orta+arka bogie motorlarini ortak kontrol eden dort BTS kanalini yonetir.
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

  // Sol on teker kanalini surer.
  bool setLeftFront(int16_t speed);

  // Sol orta ve arka BTS'lerin ortak kontrol hattini surer.
  bool setLeftBogie(int16_t speed);

  // Sag on teker kanalini surer.
  bool setRightFront(int16_t speed);

  // Sag orta ve arka BTS'lerin ortak kontrol hattini surer.
  bool setRightBogie(int16_t speed);

  // Dort kontrol grubunu ayri PWM degerleriyle surer.
  bool setDriveGroupSpeeds(int16_t leftFrontSpeed,
                           int16_t leftBogieSpeed,
                           int16_t rightFrontSpeed,
                           int16_t rightBogieSpeed);

  // Diferansiyel surus uyumlulugu icin ayni taraftaki on ve bogie grubuna
  // ayni hiz degerini uygular.
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
