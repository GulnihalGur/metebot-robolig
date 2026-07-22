#pragma once

#include <Arduino.h>
#include <Pins.h>

// UART uzerinden satir tabanli haberlesmeyi yonetir.
class UARTLink {
public:
  explicit UARTLink(Stream& stream = Serial);

  void begin(uint32_t baud = Pins::LINK_BAUD);

  // Gelen satiri String olarak verir; dahili tampon sabittir.
  bool readLine(String& outLine);

  void sendLine(const String& line);
  void sendLine(const char* line);
  void sendLine(const char* first, const char* second);

  void sendOk(const String& message);
  void sendOk(const char* message = "OK");
  void sendOk(const char* first, const char* second);

  void sendError(const String& message);
  void sendError(const char* message);
  void sendError(const char* first, const char* second);
  void sendError(const char* first, const String& second);

private:
  static constexpr size_t BUFFER_SIZE = 96;

  Stream* _stream;
  char _buffer[BUFFER_SIZE + 1];
  size_t _bufferLength;
};
