#include "Display.h"

// Kullanilacak OLED nesnesini kaydeder.
Display::Display(OLED& oled)
  : _oled(&oled) {}

// Donanimlarin baslangic durumunu ekranda gosterir.
void Display::boot(bool motorOk, bool servoOk, bool rfidOk, bool powerOk) {
  _oled->status(
    String("Motor:") + (motorOk ? "OK" : "ERR"),
    String("Servo:") + (servoOk ? "OK" : "ERR") +
    " RFID:" + (rfidOk ? "OK" : "ERR"),
    String("Power:") + (powerOk ? "OK" : "ADC yok")
  );
}

// Joystick ve motor PWM degerlerini ekranda gosterir.
void Display::joystick(const JoystickPacket& packet,
                       int16_t leftPwm,
                       int16_t rightPwm) {
  // Ekran basligini ayarlar.
  _oled->title("JOYSTICK");

  // Joystick X ve Y yuzdelerini yazar.
  _oled->printLine(
    2,
    "X:" + String(packet.xPercent) +
    " Y:" + String(packet.yPercent)
  );

  // Sol ve sag motor PWM degerlerini yazar.
  _oled->printLine(
    3,
    "L:" + String(leftPwm) +
    " R:" + String(rightPwm)
  );

  // Basili buton bilgisini yazar.
  _oled->printLine(4, "Btn:" + String(packet.buttons));

  // Bilgileri fiziksel ekrana aktarir.
  _oled->show();
}

// Okunan RFID UID bilgisini gosterir.
void Display::rfid(const String& uid) {
  _oled->uid(uid);
}

// Lineer aktuator durumunu gosterir.
void Display::actuator(const String& state) {
  _oled->status("Actuator", state);
}

// Fail-safe nedenini uyari olarak gosterir.
void Display::failSafe(const String& reason) {
  _oled->warning("FAILSAFE " + reason);
}
