#pragma once

#include <Arduino.h>
#include <Pins.h>

// UART uzerinden satir tabanli haberlesmeyi yonetir.
class UARTLink {
public:
  // Kullanilacak seri haberlesme nesnesini alir.
  explicit UARTLink(Stream& stream = Serial);

  // Seri haberlesmeyi verilen baud degeriyle baslatir.
  void begin(uint32_t baud = Pins::LINK_BAUD);

  // Satir sonu \n olan komutu okur.
  bool readLine(String& outLine);

  // Normal mesaj gonderir.
  void sendLine(const String& line);

  // Basarili durum mesaji gonderir.
  void sendOk(const String& message = "OK");

  // Hata mesaji gonderir.
  void sendError(const String& message);

private:
  // Kullanilan seri haberlesme nesnesi.
  Stream* _stream;

  // Gelen karakterleri gecici olarak tutar.
  String _buffer;
};
