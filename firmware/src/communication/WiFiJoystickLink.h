#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// PC'den Wi-Fi/UDP ile gelen joystick satirlarini alir.
class WiFiJoystickLink {
public:
  WiFiJoystickLink();

  // Deneyap'i erisim noktasi olarak baslatir ve UDP portunu dinler.
  bool begin(const char* ssid, const char* password, uint16_t port);

  // Yeni UDP paketi varsa metin satiri olarak dondurur.
  bool readLine(String& outLine);

  bool ready() const;
  IPAddress localIp() const;

private:
  static constexpr size_t BUFFER_SIZE = 128;

  WiFiUDP _udp;
  uint16_t _port;
  bool _ready;
  char _buffer[BUFFER_SIZE + 1];
};
