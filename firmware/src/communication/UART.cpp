#include "UART.h"

// Kullanilacak seri haberlesme nesnesini kaydeder.
UARTLink::UARTLink(Stream& stream)
  : _stream(&stream), _buffer("") {}

// Seri haberlesmeyi baslatir.
void UARTLink::begin(uint32_t baud) {
  // Yalnizca ana Serial portuysa baud ayari yapar.
  if (&Serial == _stream) {
    Serial.begin(baud);
    delay(100);
  }
}

// UART uzerinden bir satir okumaya calisir.
bool UARTLink::readLine(String& outLine) {
  // Gelen tum karakterleri sirayla okur.
  while (_stream->available() > 0) {
    char c = static_cast<char>(_stream->read());

    // Satir basi karakterini yok sayar.
    if (c == '\r') continue;

    // Satir sonu gelince mesaji tamamlar.
    if (c == '\n') {
      outLine = _buffer;
      outLine.trim();
      _buffer = "";

      // Bos olmayan satir icin true dondurur.
      return outLine.length() > 0;
    }

    // Tampon dolu degilse karakteri ekler.
    if (_buffer.length() < 96) {
      _buffer += c;
    } else {
      // Cok uzun mesajda tamponu temizler.
      _buffer = "";
      sendError("UART_LINE_TOO_LONG");
    }
  }

  // Tam satir okunmadiysa false dondurur.
  return false;
}

// Mesaji satir sonuyla birlikte gonderir.
void UARTLink::sendLine(const String& line) {
  _stream->println(line);
}

// Basarili durum mesaji gonderir.
void UARTLink::sendOk(const String& message) {
  _stream->print("OK,");
  _stream->println(message);
}

// Hata mesaji gonderir.
void UARTLink::sendError(const String& message) {
  _stream->print("ERR,");
  _stream->println(message);
}
