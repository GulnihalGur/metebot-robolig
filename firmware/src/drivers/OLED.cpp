#include "OLED.h"

// OLED ekran nesnesini ve varsayilan durumu ayarlar.
OLED::OLED()
  : _display(Pins::OLED_WIDTH, Pins::OLED_HEIGHT, &Wire, -1),
    _ready(false) {}

// I2C hattini ve OLED ekrani baslatir.
bool OLED::begin(uint8_t address, int sdaPin, int sclPin) {
  // I2C pinlerini baslatir.
  Wire.begin(sdaPin, sclPin);

  // OLED ekrani verilen adresle baslatir.
  _ready = _display.begin(SSD1306_SWITCHCAPVCC, address);
  if (!_ready) return false;

  // Baslangic mesajini ekrana yazar.
  clear();
  title("METEBOT");
  printLine(2, "Beyza HW ready");
  show();

  return true;
}

// Ekran tamponunu temizler.
void OLED::clear() {
  if (!_ready) return;

  _display.clearDisplay();
  _display.setTextColor(SSD1306_WHITE);
  _display.setTextSize(1);
  _display.setCursor(0, 0);
}

// Tampondaki goruntuyu ekrana aktarir.
void OLED::show() {
  if (!_ready) return;
  _display.display();
}

// Belirtilen satira metin yazar.
void OLED::printLine(uint8_t line, const String& text, uint8_t textSize) {
  if (!_ready) return;

  // Satirin dikey konumunu hesaplar.
  uint8_t y = line * 10;

  _display.setTextSize(textSize);
  _display.setCursor(0, y);
  _display.print(text);
}

// Ekrana baslik ve alt cizgi yazar.
void OLED::title(const String& text) {
  if (!_ready) return;

  clear();
  _display.setTextSize(1);
  _display.setCursor(0, 0);
  _display.print(text);

  // Basligin altina ayirici cizgi cizer.
  _display.drawLine(0, 10, Pins::OLED_WIDTH - 1, 10, SSD1306_WHITE);
}

// Uc satirlik durum bilgisini gosterir.
void OLED::status(const String& line1, const String& line2, const String& line3) {
  if (!_ready) return;

  title("METEBOT STATUS");
  printLine(2, line1);
  printLine(3, line2);
  printLine(4, line3);
  show();
}

// Uyari mesajini ekrana yazar.
void OLED::warning(const String& message) {
  if (!_ready) return;

  title("UYARI");
  printLine(2, message);
  show();
}

// Okunan RFID UID bilgisini gosterir.
void OLED::uid(const String& uidText) {
  if (!_ready) return;

  title("RFID UID");

  // UID yoksa kart bulunamadi mesaji yazar.
  printLine(2, uidText.length() ? uidText : "Kart yok");
  show();
}

// Ham Adafruit ekran nesnesine erisim verir.
Adafruit_SSD1306& OLED::raw() {
  return _display;
}

// OLED ekran hazir mi kontrol eder.
bool OLED::ready() const {
  return _ready;
}
