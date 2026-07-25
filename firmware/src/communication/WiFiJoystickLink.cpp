#include "WiFiJoystickLink.h"

WiFiJoystickLink::WiFiJoystickLink()
  : _port(0), _ready(false), _buffer{0} {}

bool WiFiJoystickLink::begin(const char* ssid,
                             const char* password,
                             uint16_t port) {
  if (ssid == nullptr || password == nullptr || port == 0) {
    return false;
  }

  WiFi.mode(WIFI_AP);

  if (!WiFi.softAP(ssid, password)) {
    _ready = false;
    return false;
  }

  _port = port;
  _ready = _udp.begin(_port) == 1;
  return _ready;
}

bool WiFiJoystickLink::readLine(String& outLine) {
  if (!_ready) return false;

  const int packetSize = _udp.parsePacket();
  if (packetSize <= 0) return false;

  const int readable = min(packetSize, static_cast<int>(BUFFER_SIZE));
  const int length = _udp.read(_buffer, readable);

  // Paket tamponu asarsa kalan baytlari temizle.
  while (_udp.available() > 0) {
    _udp.read();
  }

  if (length <= 0) return false;

  _buffer[length] = '\0';
  outLine = _buffer;
  outLine.trim();
  return !outLine.isEmpty();
}

bool WiFiJoystickLink::ready() const {
  return _ready;
}

IPAddress WiFiJoystickLink::localIp() const {
  return WiFi.softAPIP();
}
