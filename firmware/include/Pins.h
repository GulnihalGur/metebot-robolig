#pragma once

#include <Arduino.h>

namespace Pins {

static constexpr int8_t UNUSED_PIN = -1;

//OLED / I2C pin atamalari
static constexpr int8_t OLED_SDA = 21;
static constexpr int8_t OLED_SCL = 20;
static constexpr uint8_t OLED_I2C_ADDRESS = 0x3C;
static constexpr uint16_t OLED_WIDTH = 128;
static constexpr uint16_t OLED_HEIGHT = 64;

//PN532 RFID / UART pin atamalari
static constexpr int8_t RFID_RX = 28;          // Deneyap RX, PN532 TX 
static constexpr int8_t RFID_TX = 29;          // Deneyap TX, PN532 RX 
static constexpr uint32_t RFID_BAUD = 115200;

//Servo PWM pin atamalari
struct ServoPin {
  const char* name;
  uint8_t pin;
  uint8_t minAngle;
  uint8_t maxAngle;
  uint8_t startAngle;
};

static constexpr uint8_t SERVO_COUNT = 4;
static constexpr ServoPin SERVO_PINS[SERVO_COUNT] = {
  {"main_gripper", 10, 0, 180, 90},  // Deneyap D14
  {"extra_1",      24, 0, 180, 90},  // MOSI
  {"extra_2",      25, 0, 180, 90},  // MISO
  {"extra_3",      26, 0, 180, 90}   // SCK
};

//BTS7960 PWM pin atamalari
struct BtsPins {
  const char* name;
  int8_t rpwm;
  int8_t lpwm;
  bool invertDirection;
};

static constexpr uint8_t TOTAL_BTS7960_COUNT = 8;

static constexpr uint8_t BTS_COUNT = 6;
static constexpr BtsPins BTS_PINS[BTS_COUNT] = {
  {"BTS1_left_front_motor",   31, 30, false}, // D0 / D1
  {"BTS2_left_middle_motor",  27, 23, false}, // D4 / D8
  {"BTS3_left_rear_motor",    22, 12, false}, // D9 / D12
  {"BTS4_right_front_motor",  11,  2, false}, // D13 / A0
  {"BTS5_right_middle_motor",  3,  4, false}, // A1 / A2
  {"BTS6_right_rear_motor",    5,  6, false}  // A3 / A4
};

//7. BTS7960 lineer aktuator icin ayrildi fakat su an pini belli degil
static constexpr BtsPins LINEAR_ACTUATOR_PINS = {
  "BTS7_linear_actuator_PIN_UNASSIGNED",
  UNUSED_PIN,
  UNUSED_PIN,
  false
};

//8. BTS7960 yedek, su anda kullanilmiyor
static constexpr BtsPins SPARE_BTS8_PINS = {
  "BTS8_spare_driver_UNASSIGNED",
  UNUSED_PIN,
  UNUSED_PIN,
  false
};

//
//Sag taraf ters donuyorsa M+ / M- kablolarini degistirin veya ilgili
//invertDirection degerini true yapin.
static constexpr uint8_t DRIVE_LEFT_FRONT_INDEX    = 0;
static constexpr uint8_t DRIVE_LEFT_MIDDLE_INDEX   = 1;
static constexpr uint8_t DRIVE_LEFT_REAR_INDEX     = 2;
static constexpr uint8_t DRIVE_RIGHT_FRONT_INDEX   = 3;
static constexpr uint8_t DRIVE_RIGHT_MIDDLE_INDEX  = 4;
static constexpr uint8_t DRIVE_RIGHT_REAR_INDEX    = 5;

static constexpr uint8_t DRIVE_LEFT_INDICES[3] = {
  DRIVE_LEFT_FRONT_INDEX,
  DRIVE_LEFT_MIDDLE_INDEX,
  DRIVE_LEFT_REAR_INDEX
};

static constexpr uint8_t DRIVE_RIGHT_INDICES[3] = {
  DRIVE_RIGHT_FRONT_INDEX,
  DRIVE_RIGHT_MIDDLE_INDEX,
  DRIVE_RIGHT_REAR_INDEX
};

//Link / joystick haberlesmesi
//joystick verisi PC'den USB seri portuyla geliyorsa seri haberlesme kullanilacak.
//harici UART modulu icin ilerde kullanilacak pinler
static constexpr int8_t LINK_RX = UNUSED_PIN;
static constexpr int8_t LINK_TX = UNUSED_PIN;
static constexpr uint32_t LINK_BAUD = 115200;

//Guc izleme
//Pin tablosunda batarya ADC olcum pini yok. Eklenirse bu deger degistirilecek.
static constexpr int8_t BATTERY_ADC = UNUSED_PIN;

//varsayilanlar
static constexpr int16_t MOTOR_PWM_MAX = 255;
static constexpr uint16_t JOYSTICK_TIMEOUT_MS = 500;
static constexpr uint8_t JOYSTICK_DEADZONE_PERCENT = 8;

} //namespace Pins
