#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Pins.h>

// SSD1306 OLED ekrani kontrol eden sinif.
class OLED {
public:
  OLED();

  // I2C hattini ve OLED ekrani baslatir.
  bool begin(uint8_t address = Pins::OLED_I2C_ADDRESS,
             int sdaPin = Pins::OLED_SDA,
             int sclPin = Pins::OLED_SCL);

  // Ekran tamponunu temizler.
  void clear();

  // Tampondaki goruntuyu ekrana aktarir.
  void show();

  // Belirtilen satira metin yazar.
  void printLine(uint8_t line, const String& text, uint8_t textSize = 1);

  // Baslik ve alt cizgi olusturur.
  void title(const String& text);

  // Durum bilgilerini uc satir halinde gosterir.
  void status(const String& line1, const String& line2 = "", const String& line3 = "");

  // Uyari mesajini gosterir.
  void warning(const String& message);

  // RFID UID bilgisini gosterir.
  void uid(const String& uidText);

  // Ham ekran nesnesine erisim verir.
  Adafruit_SSD1306& raw();

  // Ekran hazir mi kontrol eder.
  bool ready() const;

private:
  // SSD1306 ekran nesnesi.
  Adafruit_SSD1306 _display;

  // Baslatma durumu.
  bool _ready;
};
