#pragma once

#include <Arduino.h>
#include <drivers/OLED/OLED.h>
#include <communication/Joystick/Joystick.h>

// OLED ekran ciktilarini yoneten sinif.
class Display {
public:
  // Kullanilacak OLED nesnesini alir.
  explicit Display(OLED& oled);

  // Donanimlarin baslangic durumunu gosterir.
  void boot(bool motorOk, bool servoOk, bool rfidOk, bool powerOk);

  // Joystick ve motor PWM bilgilerini gosterir.
  void joystick(const JoystickPacket& packet,
                int16_t leftPwm,
                int16_t rightPwm);

  // RFID UID bilgisini gosterir.
  void rfid(const String& uid);

  // Lineer aktuator durumunu gosterir.
  void actuator(const String& state);

  // Fail-safe nedenini uyari olarak gosterir.
  void failSafe(const String& reason);

private:
  // Kullanilan OLED nesnesinin adresi.
  OLED* _oled;
};
