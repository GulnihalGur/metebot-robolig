#include "Display.h"

Display::Display(OLED& oled)
  : _oled(&oled) {}

void Display::boot(bool motorOk, bool servoOk, bool rfidOk, bool powerOk) {
  char line1[24];
  char line2[32];
  char line3[24];

  snprintf(line1, sizeof(line1), "Motor:%s", motorOk ? "OK" : "ERR");
  snprintf(
    line2,
    sizeof(line2),
    "Servo:%s RFID:%s",
    servoOk ? "OK" : "ERR",
    rfidOk ? "OK" : "ERR"
  );
  snprintf(line3, sizeof(line3), "Power:%s", powerOk ? "OK" : "ADC yok");

  _oled->status(line1, line2, line3);
}

void Display::joystick(const JoystickPacket& packet,
                       int16_t leftPwm,
                       int16_t rightPwm) {
  char axisLine[28];
  char motorLine[28];
  char buttonLine[24];

  snprintf(
    axisLine,
    sizeof(axisLine),
    "X:%d Y:%d",
    packet.xPercent,
    packet.yPercent
  );

  snprintf(
    motorLine,
    sizeof(motorLine),
    "L:%d R:%d",
    leftPwm,
    rightPwm
  );

  snprintf(
    buttonLine,
    sizeof(buttonLine),
    "Btn:%lu",
    static_cast<unsigned long>(packet.buttons)
  );

  _oled->title("JOYSTICK");
  _oled->printLine(2, axisLine);
  _oled->printLine(3, motorLine);
  _oled->printLine(4, buttonLine);
  _oled->show();
}

void Display::rfid(const String& uid) {
  _oled->uid(uid);
}

void Display::actuator(const String& state) {
  _oled->status("Actuator", state);
}

void Display::failSafe(const String& reason) {
  char message[48];
  snprintf(message, sizeof(message), "FAILSAFE %s", reason.c_str());
  _oled->warning(message);
}
