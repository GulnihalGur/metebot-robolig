#pragma once

#include <Arduino.h>

namespace Pins {

static constexpr int8_t UNUSED_PIN = -1;

// -----------------------------------------------------------------------------
// ONEMLI PIN KURALI
// Kodda kart uzerindeki fiziksel sira numaralari (ornegin 31, 30, 27) degil,
// Deneyap Kart 1A'nin Arduino pin adlari kullanilir (D0, D1, D4, A4 vb.).
// Fiziksel pin numaralari yalnizca baglanti dokumaninda referans olarak verilir.
// -----------------------------------------------------------------------------

// OLED / I2C pin atamalari
static constexpr int8_t OLED_SDA = SDA;  // Fiziksel pin 21
static constexpr int8_t OLED_SCL = SCL;  // Fiziksel pin 20
static constexpr uint8_t OLED_I2C_ADDRESS = 0x3C;
static constexpr uint16_t OLED_WIDTH = 128;
static constexpr uint16_t OLED_HEIGHT = 64;

// PN532 RFID / UART pin atamalari
// USB seri haberlesmesiyle cakismamasi icin kartin TX/RX pinleri kullanilmaz.
// ESP32 UART pin matrisi sayesinde Serial2, DAC1 ve DAC2 pinlerine yonlendirilir.
static constexpr int8_t RFID_RX = DAC1;  // Fiziksel pin 8, PN532 TX -> Deneyap DAC1
static constexpr int8_t RFID_TX = DAC2;  // Fiziksel pin 9, PN532 RX -> Deneyap DAC2
static constexpr uint32_t RFID_BAUD = 115200;

// Servo PWM pin atamalari
struct ServoPin {
  const char* name;
  uint8_t pin;
  uint8_t minAngle;
  uint8_t maxAngle;
  uint8_t startAngle;
};

static constexpr uint8_t SERVO_COUNT = 4;
static constexpr ServoPin SERVO_PINS[SERVO_COUNT] = {
  {"main_gripper", D14,  0, 180, 90},  // Fiziksel pin 11
  {"extra_1",      MOSI, 0, 180, 90},  // Fiziksel pin 24 / D7
  {"extra_2",      MISO, 0, 180, 90},  // Fiziksel pin 25 / D6
  {"extra_3",      SCK,  0, 180, 90}   // Fiziksel pin 26 / D5
};

// BTS7960 PWM pin atamalari
struct BtsPins {
  const char* name;
  int8_t rpwm;
  int8_t lpwm;
  bool invertDirection;
};

static constexpr uint8_t TOTAL_BTS7960_COUNT = 8;
static constexpr uint8_t PHYSICAL_DRIVE_BTS_COUNT = 6;

// Alti fiziksel surus BTS'si korunur; fakat yazilimda iki mantiksal kanal vardir.
// Sol taraftaki uc BTS ayni SOL RPWM/LPWM hatlarini, sag taraftaki uc BTS ise
// ayni SAG RPWM/LPWM hatlarini dinler. Motor guc cikislari ortaklastirilmaz.
static constexpr uint8_t BTS_COUNT = 2;
static constexpr BtsPins BTS_PINS[BTS_COUNT] = {
  {"left_drive_shared_control",  D0,  D1,  false},  // Fiziksel pin 31 / 30
  {"right_drive_shared_control", D12, D15, false}   // Fiziksel pin 13 / 10
};

// 7. BTS7960 lineer aktuator icin ayrildi.
static constexpr BtsPins LINEAR_ACTUATOR_PINS = {
  "BTS7_linear_actuator",
  D4,  // Fiziksel pin 27
  A4,  // Fiziksel pin 6
  false
};

// 8. BTS7960 yedek, su anda kullanilmiyor.
static constexpr BtsPins SPARE_BTS8_PINS = {
  "BTS8_spare_driver_UNASSIGNED",
  UNUSED_PIN,
  UNUSED_PIN,
  false
};

// Ortak kontrol nedeniyle yon tersleme taraf bazindadir. Tek bir motor ters
// donuyorsa o motorun M+ ve M- kablolari fiziksel olarak yer degistirilmelidir.
static constexpr uint8_t DRIVE_LEFT_INDEX = 0;
static constexpr uint8_t DRIVE_RIGHT_INDEX = 1;

// Link / joystick haberlesmesi
// Joystick verisi PC'den USB seri portuyla gelir.
static constexpr int8_t LINK_RX = UNUSED_PIN;
static constexpr int8_t LINK_TX = UNUSED_PIN;
static constexpr uint32_t LINK_BAUD = 115200;

// Guc izleme
// Pin tablosunda batarya ADC olcum pini yok. Eklenirse bu deger degistirilecek.
static constexpr int8_t BATTERY_ADC = UNUSED_PIN;

// Varsayilanlar
static constexpr int16_t MOTOR_PWM_MAX = 255;
static constexpr uint16_t JOYSTICK_TIMEOUT_MS = 500;
static constexpr uint8_t JOYSTICK_DEADZONE_PERCENT = 8;

}  // namespace Pins
