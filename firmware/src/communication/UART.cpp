#include "UART.h"

UARTLink::UARTLink(Stream& stream)
  : _stream(&stream), _buffer{0}, _bufferLength(0) {}

void UARTLink::begin(uint32_t baud) {
  if (&Serial == _stream) {
    Serial.begin(baud);
    delay(100);
  }
}

bool UARTLink::readLine(String& outLine) {
  while (_stream->available() > 0) {
    const char c = static_cast<char>(_stream->read());

    if (c == '\r') continue;

    if (c == '\n') {
      _buffer[_bufferLength] = '\0';
      outLine = _buffer;
      outLine.trim();
      _bufferLength = 0;
      _buffer[0] = '\0';
      return !outLine.isEmpty();
    }

    if (_bufferLength < BUFFER_SIZE) {
      _buffer[_bufferLength++] = c;
      _buffer[_bufferLength] = '\0';
    } else {
      _bufferLength = 0;
      _buffer[0] = '\0';
      sendError("UART_LINE_TOO_LONG");
    }
  }

  return false;
}

void UARTLink::sendLine(const String& line) {
  _stream->println(line);
}

void UARTLink::sendLine(const char* line) {
  _stream->println(line);
}

void UARTLink::sendLine(const char* first, const char* second) {
  _stream->print(first);
  _stream->println(second);
}

void UARTLink::sendOk(const String& message) {
  _stream->print(F("OK,"));
  _stream->println(message);
}

void UARTLink::sendOk(const char* message) {
  _stream->print(F("OK,"));
  _stream->println(message);
}

void UARTLink::sendOk(const char* first, const char* second) {
  _stream->print(F("OK,"));
  _stream->print(first);
  _stream->println(second);
}

void UARTLink::sendError(const String& message) {
  _stream->print(F("ERR,"));
  _stream->println(message);
}

void UARTLink::sendError(const char* message) {
  _stream->print(F("ERR,"));
  _stream->println(message);
}

void UARTLink::sendError(const char* first, const char* second) {
  _stream->print(F("ERR,"));
  _stream->print(first);
  _stream->println(second);
}

void UARTLink::sendError(const char* first, const String& second) {
  _stream->print(F("ERR,"));
  _stream->print(first);
  _stream->println(second);
}
