#include "ServoDriver.h"

// Baslangic degerlerini ayarlar.
ServoDriver::ServoDriver()
  : _count(0), _ready(false) {}

// Servolari pinlere baglar ve ilk acilarina getirir.
bool ServoDriver::begin(const Pins::ServoPin* pins, uint8_t countValue) {
  // Pin listesi yoksa baslatma yapma.
  if (pins == nullptr) return false;

  // Servo sayisini izin verilen en buyuk degerle sinirla.
  _count = min<uint8_t>(countValue, MAX_SERVOS);

  // ESP32 PWM zamanlayicilarini servo kullanimi icin ayir.
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  // Tum servolari tek tek hazirla.
  for (uint8_t i = 0; i < _count; i++) {
    _servos[i].config = &pins[i];                    // Servo ayarlarini kaydet.
    _servos[i].currentAngle = limitAngle(pins[i].startAngle, pins[i]); // Ilk aciyi sinirla.
    _servos[i].targetAngle = _servos[i].currentAngle; // Hedefi ilk aciya esit yap.
    _servos[i].speedDegPerSec = 120;                 // Varsayilan hizi ayarla.
    _servos[i].lastUpdateMs = millis();              // Son guncelleme zamanini kaydet.
    _servos[i].attached = false;                     // Henuz bagli degil.

    _servos[i].servo.setPeriodHertz(50);             // Standart servo frekansi.
    _servos[i].servo.attach(pins[i].pin, 500, 2500); // Servoyu pine bagla.
    _servos[i].attached = true;                      // Baglandi olarak isaretle.
    _servos[i].servo.write(_servos[i].currentAngle); // Ilk aciyi uygula.
  }

  _ready = true; // Surucu kullanima hazir.
  return true;
}

// Servoyu verilen aciya yumusak sekilde hareket ettirir.
bool ServoDriver::moveTo(uint8_t index, uint8_t targetAngle, uint16_t speedDegPerSec) {
  // Gecersiz veya bagli olmayan servo ise islemi durdur.
  if (index >= _count || !_servos[index].attached) return false;

  const Pins::ServoPin& cfg = *_servos[index].config;
  _servos[index].targetAngle = limitAngle(targetAngle, cfg);       // Hedef aciyi sinirla.
  _servos[index].speedDegPerSec = max<uint16_t>(1, speedDegPerSec); // Hizi en az 1 yap.
  return true;
}

// Servoyu beklemeden dogrudan verilen aciya getirir.
bool ServoDriver::writeNow(uint8_t index, uint8_t angleValue) {
  if (index >= _count || !_servos[index].attached) return false;

  const Pins::ServoPin& cfg = *_servos[index].config;
  uint8_t safeAngle = limitAngle(angleValue, cfg); // Acinin guvenli aralikta olmasini sagla.
  _servos[index].currentAngle = safeAngle;         // Mevcut aciyi guncelle.
  _servos[index].targetAngle = safeAngle;          // Hedef aciyi da ayni yap.
  _servos[index].servo.write(safeAngle);           // Aciyi servoya hemen yaz.
  _servos[index].lastUpdateMs = millis();          // Zamani yenile.
  return true;
}

// Secilen servonun PWM baglantisini keser.
bool ServoDriver::detach(uint8_t index) {
  if (index >= _count || !_servos[index].attached) return false;
  _servos[index].servo.detach();
  _servos[index].attached = false;
  return true;
}

// Tum servolarin PWM baglantisini keser.
void ServoDriver::detachAll() {
  for (uint8_t i = 0; i < _count; i++) {
    detach(i);
  }
}

// Servo hareketlerini bloklamadan adim adim gunceller.
void ServoDriver::update() {
  uint32_t now = millis(); // Su anki zamani al.

  for (uint8_t i = 0; i < _count; i++) {
    ServoState& s = _servos[i];

    // Servo bagli degilse veya hedefe ulastiysa atla.
    if (!s.attached || s.currentAngle == s.targetAngle) continue;

    uint32_t elapsed = now - s.lastUpdateMs; // Gecen sureyi hesapla.
    if (elapsed < 15) continue;              // Cok sik guncelleme yapma.

    // Hiza ve gecen sureye gore kac derece ilerleyecegini hesapla.
    uint16_t step = max<uint16_t>(1, (static_cast<uint32_t>(s.speedDegPerSec) * elapsed) / 1000UL);

    if (s.currentAngle < s.targetAngle) {
      // Aciyi hedefe dogru arttir.
      s.currentAngle = static_cast<uint8_t>(min<int>(s.targetAngle, static_cast<int>(s.currentAngle) + static_cast<int>(step)));
    } else {
      // Aciyi hedefe dogru azalt.
      s.currentAngle = (step >= (s.currentAngle - s.targetAngle))
                         ? s.targetAngle
                         : static_cast<uint8_t>(s.currentAngle - step);
    }

    s.servo.write(s.currentAngle); // Yeni aciyi servoya yaz.
    s.lastUpdateMs = now;          // Guncelleme zamanini kaydet.
  }
}

// Servonun mevcut acisini dondurur.
uint8_t ServoDriver::angle(uint8_t index) const {
  if (index >= _count) return 0;
  return _servos[index].currentAngle;
}

// Servonun hedef acisini dondurur.
uint8_t ServoDriver::target(uint8_t index) const {
  if (index >= _count) return 0;
  return _servos[index].targetAngle;
}

// Tanimli servo sayisini dondurur.
uint8_t ServoDriver::count() const {
  return _count;
}

// Surucunun hazir olup olmadigini dondurur.
bool ServoDriver::ready() const {
  return _ready;
}

// Aciyi servonun izin verilen araliginda tutar.
uint8_t ServoDriver::limitAngle(uint8_t value, const Pins::ServoPin& cfg) {
  return constrain(value, cfg.minAngle, cfg.maxAngle);
}
