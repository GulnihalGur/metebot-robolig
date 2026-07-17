#include "Joystick.h"

// Varsayilan joystick verilerini ayarlar.
Joystick::Joystick() {
  _packet.xPercent = 0;
  _packet.yPercent = 0;
  _packet.twistPercent = 0;
  _packet.throttlePercent = 0;
  _packet.hatAngle = -1;
  _packet.buttons = 0;
  _packet.timestampMs = 0;
  _packet.valid = false;
}

// Gelen joystick satirini ayristirir.
bool Joystick::parseLine(const String& inputLine) {
  String line = inputLine;
  line.trim();

  // Komut JOY ile baslamiyorsa reddeder.
  if (!startsWithCommand(line, "JOY")) return false;

  String fields[7];

  // CSV alanlarini ayirir.
  uint8_t fieldCount = splitCsv(line, fields, 7);

  // Eski format: JOY,x,y,buttons
  if (fieldCount == 4) {
    long rawX = fields[1].toInt();
    long rawY = fields[2].toInt();

    // X eksenini yuzdeye cevirir.
    _packet.xPercent =
      applyDeadzone(normalizeAxis(rawX), Pins::JOYSTICK_DEADZONE_PERCENT);

    // Y eksenini yuzdeye cevirir.
    _packet.yPercent =
      applyDeadzone(normalizeAxis(rawY), Pins::JOYSTICK_DEADZONE_PERCENT);

    // Eski formatta olmayan degerleri varsayilan yapar.
    _packet.twistPercent = 0;
    _packet.throttlePercent = 100;
    _packet.hatAngle = -1;

    _packet.buttons = static_cast<uint32_t>(fields[3].toInt());
    _packet.timestampMs = millis();
    _packet.valid = true;

    return true;
  }

  // Yeni format tam 7 alan olmali.
  if (fieldCount != 7) return false;

  long rawX = fields[1].toInt();
  long rawY = fields[2].toInt();
  long rawTwist = fields[3].toInt();
  long rawThrottle = fields[4].toInt();
  long rawHat = fields[5].toInt();

  // X eksenini normalize eder.
  _packet.xPercent =
    applyDeadzone(normalizeAxis(rawX), Pins::JOYSTICK_DEADZONE_PERCENT);

  // Y eksenini normalize eder.
  _packet.yPercent =
    applyDeadzone(normalizeAxis(rawY), Pins::JOYSTICK_DEADZONE_PERCENT);

  // Twist eksenini normalize eder.
  _packet.twistPercent =
    applyDeadzone(normalizeAxis(rawTwist), Pins::JOYSTICK_DEADZONE_PERCENT);

  // Gaz kolunu yuzdeye cevirir.
  _packet.throttlePercent = normalizeThrottle(rawThrottle);

  // Hat switch kullanilmiyorsa -1 tutar.
  if (rawHat < 0) {
    _packet.hatAngle = -1;
  } else {
    _packet.hatAngle = static_cast<int16_t>(constrain(rawHat, 0L, 359L));
  }

  // Buton bit maskesini kaydeder.
  _packet.buttons = static_cast<uint32_t>(fields[6].toInt());

  // Son paket zamanini kaydeder.
  _packet.timestampMs = millis();
  _packet.valid = true;

  return true;
}

// Sol ve sag motor PWM degerlerini hesaplar.
DifferentialOutput Joystick::differential(uint8_t maxPwm) const {
  DifferentialOutput out{0, 0};

  // Gecerli paket yoksa motorlari durdurur.
  if (!_packet.valid) return out;

  // Y eksenini ileri geri harekete cevirir.
  int16_t forward =
    map(_packet.yPercent, -100, 100, -maxPwm, maxPwm);

  // X eksenini donus degerine cevirir.
  int16_t turn =
    map(_packet.xPercent, -100, 100, -maxPwm, maxPwm);

  int16_t left = forward + turn;
  int16_t right = forward - turn;

  // En buyuk PWM degerini bulur.
  int16_t largest = max(abs(left), abs(right));

  // Orani bozmadan PWM degerlerini sinirlar.
  if (largest > maxPwm && largest > 0) {
    left = static_cast<int16_t>(
      (static_cast<int32_t>(left) * maxPwm) / largest
    );

    right = static_cast<int16_t>(
      (static_cast<int32_t>(right) * maxPwm) / largest
    );
  }

  out.leftPwm = constrain(left, -255, 255);
  out.rightPwm = constrain(right, -255, 255);

  return out;
}

// Gaz koluna gore maksimum PWM hesaplar.
uint8_t Joystick::throttlePwm(uint8_t minPwm, uint8_t maxPwm) const {
  // Min ve max ters girildiyse yer degistirir.
  if (minPwm > maxPwm) {
    uint8_t temp = minPwm;
    minPwm = maxPwm;
    maxPwm = temp;
  }

  return static_cast<uint8_t>(
    map(_packet.throttlePercent, 0, 100, minPwm, maxPwm)
  );
}

// Belirtilen buton basili mi kontrol eder.
bool Joystick::buttonPressed(uint8_t buttonIndex) const {
  if (buttonIndex >= 32) return false;

  return (_packet.buttons & (1UL << buttonIndex)) != 0;
}

// Joystick verisi zaman asimina ugradi mi kontrol eder.
bool Joystick::timedOut(uint16_t timeoutMs) const {
  if (!_packet.valid) return true;

  return (millis() - _packet.timestampMs) > timeoutMs;
}

// Son joystick paketini dondurur.
const JoystickPacket& Joystick::packet() const {
  return _packet;
}

// Satir belirtilen komutla basliyor mu kontrol eder.
bool Joystick::startsWithCommand(const String& line, const char* command) {
  String upper = line;
  upper.toUpperCase();

  String cmd = command;
  cmd.toUpperCase();

  return upper.startsWith(cmd + ",") || upper == cmd;
}

// CSV satirini parcalara ayirir.
uint8_t Joystick::splitCsv(const String& line, String* fields, uint8_t maxFields) {
  if (fields == nullptr || maxFields == 0) return 0;

  uint8_t count = 0;
  int start = 0;

  while (count < maxFields) {
    int comma = line.indexOf(',', start);

    // Son alani kaydeder.
    if (comma < 0) {
      fields[count++] = line.substring(start);
      break;
    }

    fields[count++] = line.substring(start, comma);
    start = comma + 1;
  }

  return count;
}

// Eksen degerini -100 ile 100 arasina cevirir.
int16_t Joystick::normalizeAxis(long value) {
  // Deger zaten yuzdeyse dogrudan kullanir.
  if (value >= -100 && value <= 100) {
    return static_cast<int16_t>(value);
  }

  // Ham 0..1023 degerini yuzdeye cevirir.
  if (value >= 0 && value <= 1023) {
    return static_cast<int16_t>(
      map(value, 0, 1023, -100, 100)
    );
  }

  // Gecersiz degeri sinirlar.
  return static_cast<int16_t>(
    constrain(value, -100L, 100L)
  );
}

// Gaz kolunu 0 ile 100 arasina cevirir.
uint8_t Joystick::normalizeThrottle(long value) {
  // Deger zaten yuzdeyse dogrudan kullanir.
  if (value >= 0 && value <= 100) {
    return static_cast<uint8_t>(value);
  }

  // Ham 0..1023 degerini yuzdeye cevirir.
  if (value >= 0 && value <= 1023) {
    return static_cast<uint8_t>(
      map(value, 0, 1023, 0, 100)
    );
  }

  // Gecersiz degeri sinirlar.
  return static_cast<uint8_t>(
    constrain(value, 0L, 100L)
  );
}

// Merkeze yakin kucuk degerleri sifirlar.
int16_t Joystick::applyDeadzone(int16_t value, uint8_t deadzonePercent) {
  if (abs(value) <= deadzonePercent) return 0;

  return value;
}
