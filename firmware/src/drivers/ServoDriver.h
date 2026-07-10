#pragma once 

#include <Arduino.h>    
#include <ESP32Servo.h> 
#include <Pins.h>       

class ServoDriver {
public:
  // Kullanilabilecek en fazla servo sayisi.
  static constexpr uint8_t MAX_SERVOS = Pins::SERVO_COUNT;

  // Sinifin kurucu fonksiyonu.
  ServoDriver();

  // Servolari baslatir ve pinlere baglar.
  bool begin(const Pins::ServoPin* pins = Pins::SERVO_PINS, uint8_t count = Pins::SERVO_COUNT);

  // Servoyu hedef aciya yumusak hareket ettirir.
  bool moveTo(uint8_t index, uint8_t targetAngle, uint16_t speedDegPerSec = 120);

  // Servoyu hedef aciya hemen getirir.
  bool writeNow(uint8_t index, uint8_t angle);

  // Tek servonun baglantisini keser.
  bool detach(uint8_t index);

  // Tum servolarin baglantisini keser.
  void detachAll();

  // loop() icinde surekli cagrilir, hareketi bloklamadan yumusatir.
  void update();

  // Mevcut aciyi dondurur.
  uint8_t angle(uint8_t index) const;

  // Hedef aciyi dondurur.
  uint8_t target(uint8_t index) const;

  // Servo sayisini dondurur.
  uint8_t count() const;

  // Baslatma durumunu dondurur.
  bool ready() const;

private:
  // Her servo icin tutulan bilgiler.
  struct ServoState {
    Servo servo;                    // Servo nesnesi.
    const Pins::ServoPin* config;   // Pin ve aci ayarlari.
    uint8_t currentAngle;           // Mevcut aci.
    uint8_t targetAngle;            // Hedef aci.
    uint16_t speedDegPerSec;        // Saniyedeki hareket derecesi.
    uint32_t lastUpdateMs;          // Son guncelleme zamani.
    bool attached;                  // Servo bagli mi?
  };

  ServoState _servos[MAX_SERVOS]; // Servo durum dizisi.
  uint8_t _count;                 // Aktif servo sayisi.
  bool _ready;                    // Surucu hazir mi?

  // Aciyi minimum ve maksimum deger arasinda tutar.
  static uint8_t limitAngle(uint8_t value, const Pins::ServoPin& cfg);
};
