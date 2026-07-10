#include "RFID.h"

// Seri portu ve PN532 nesnesini hazirlar.
RFID::RFID(HardwareSerial& serialPort)
  : _serial(&serialPort),
    _pn532(&serialPort),
    _ready(false),
    _firmwareVersion(0) {}

// RFID modulunu baslatir.
bool RFID::begin(uint32_t baud, int rxPin, int txPin) {
  // UART haberlesmesini baslatir.
  _serial->begin(baud, SERIAL_8N1, rxPin, txPin);
  delay(50);

  // PN532 modulunu baslatir.
  _pn532.begin();

  // Modulun firmware bilgisini okur.
  _firmwareVersion = _pn532.getFirmwareVersion();

  // Modul bulunamazsa hata dondurur.
  if (_firmwareVersion == 0) {
    _ready = false;
    return false;
  }

  // Kart okuma modunu ayarlar.
  _pn532.SAMConfig();

  // Okumanin uzun sure bloklanmasini azaltir.
  _pn532.setPassiveActivationRetries(0x01);

  _ready = true;
  return true;
}

// RFID kartin UID bilgisini okur.
bool RFID::readUID(uint8_t* uidBuffer, uint8_t* uidLength, uint16_t timeoutMs) {
  // Modul veya adresler gecersizse cikilir.
  if (!_ready || uidBuffer == nullptr || uidLength == nullptr) return false;

  // UID uzunlugunu sifirlar.
  *uidLength = 0;

  // ISO14443A uyumlu karti okumaya calisir.
  bool success = _pn532.readPassiveTargetID(PN532_MIFARE_ISO14443A,
                                            uidBuffer,
                                            uidLength,
                                            timeoutMs);

  // Okuma basarisizsa UID bilgisini temizler.
  if (!success || *uidLength == 0 || *uidLength > MAX_UID_LENGTH) {
    *uidLength = 0;
    return false;
  }

  return true;
}

// UID bilgisini okunabilir String olarak dondurur.
String RFID::readUIDString(uint16_t timeoutMs) {
  uint8_t uid[MAX_UID_LENGTH];
  uint8_t uidLength = 0;

  // Kart okunamazsa bos String dondurur.
  if (!readUID(uid, &uidLength, timeoutMs)) {
    return "";
  }

  // UID byte dizisini metne cevirir.
  return uidToString(uid, uidLength);
}

// Modulu kullanima hazir mi kontrol eder.
bool RFID::ready() const {
  return _ready;
}

// Firmware surum bilgisini dondurur.
uint32_t RFID::firmwareVersion() const {
  return _firmwareVersion;
}

// UID byte dizisini HEX metne cevirir.
String RFID::uidToString(const uint8_t* uid, uint8_t uidLength) {
  // Gecersiz UID icin bos String dondurur.
  if (uid == nullptr || uidLength == 0) return "";

  String result;

  for (uint8_t i = 0; i < uidLength; i++) {
    // Tek haneli HEX degerlerin basina sifir ekler.
    if (uid[i] < 0x10) result += "0";

    // Byte degerini HEX olarak ekler.
    result += String(uid[i], HEX);

    // Byte degerleri arasina iki nokta ekler.
    if (i + 1 < uidLength) result += ":";
  }

  // Harfleri buyuk harfe cevirir.
  result.toUpperCase();
  return result;
}
