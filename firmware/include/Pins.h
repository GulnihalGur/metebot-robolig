#pragma once

#include <Arduino.h>

namespace Pins {

static constexpr int8_t UNUSED_PIN = -1;

// -----------------------------------------------------------------------------
// KART: DENEYAP KART 1A v2
//
// Kodda kart uzerindeki etiketler kullanilir: D0, D1, A0, A1, SDA, SCL vb.
// Fiziksel header sira numaralari veya dogrudan GPIO numaralari kullanilmaz.
//
// D15 dahili GPKEY butonuna baglidir ve bu projede kullanilmaz.
// D9 dahili RGB LED'e baglidir ve bu projede kullanilmaz.
// D8 SD kart hatti ile cakisma riski nedeniyle bos birakilmistir.
// -----------------------------------------------------------------------------

// OLED / I2C
// SDA ayni zamanda D10, SCL ayni zamanda D11 olarak adlandirilir.
static constexpr int8_t OLED_SDA = SDA;
static constexpr int8_t OLED_SCL = SCL;
static constexpr uint8_t OLED_I2C_ADDRESS = 0x3C;
static constexpr uint16_t OLED_WIDTH = 128;
static constexpr uint16_t OLED_HEIGHT = 64;

// PN532 RFID / UART
// PN532 TX -> Deneyap RX (RFID_RX) D3
// PN532 RX -> Deneyap TX (RFID_TX) D2
static constexpr int8_t RFID_TX = RX; // fiziksel pin 28 yani D3
static constexpr int8_t RFID_RX = TX; // fiziksel pin 29 yani D2
static constexpr uint32_t RFID_BAUD = 115200;

// Servo PWM pinleri
struct ServoPin {
  const char* name;
  uint8_t pin;
  uint8_t minAngle;
  uint8_t maxAngle;
  uint8_t startAngle;
};

static constexpr uint8_t SERVO_COUNT = 4;
static constexpr ServoPin SERVO_PINS[SERVO_COUNT] = {
  {"main_gripper", D14, 0, 180, 90},
  {"extra_1",      MOSI, 0, 180, 90},  // MOSI = D7
  {"extra_2",      MISO, 0, 180, 90},  // MISO = D6
  {"extra_3",      SCK,  0, 180, 90}   // SCK  = D5
};

// BTS7960 kontrol pinleri
struct BtsPins {
  const char* name;
  int8_t rpwm;
  int8_t lpwm;
  bool invertDirection;
};

static constexpr uint8_t TOTAL_BTS7960_COUNT = 8;
static constexpr uint8_t PHYSICAL_DRIVE_BTS_COUNT = 6;

// Surus sistemi dort mantiksal kontrol grubuna ayrilir:
// 1) sol on teker, 2) sol orta+arka bogie,
// 3) sag on teker, 4) sag orta+arka bogie.
//
// Orta ve arka motorlarin BTS kontrol girisleri ortaklastirilir.
// Her motor kendi BTS7960 surucusunu ve kendi M+/M- cikisini kullanmaya devam eder.
// BTS motor cikislari birbirine baglanmaz.
static constexpr uint8_t BTS_COUNT = 4;
static constexpr BtsPins BTS_PINS[BTS_COUNT] = {
  {"left_front",        D0,  D1,  false},
  {"left_bogie_shared", A2,  A3,  false},
  {"right_front",       D12, D13, false},
  {"right_bogie_shared",A5,  A6,  false}
};

// 7. BTS7960 lineer aktuator icin kullanilir.
static constexpr BtsPins LINEAR_ACTUATOR_PINS = {
  "BTS7_linear_actuator",
  D4,
  A4,
  false
};

// 8. BTS7960 yedektir ve su anda kontrol pini atanmamistir.
static constexpr BtsPins SPARE_BTS8_PINS = {
  "BTS8_spare_driver_UNASSIGNED",
  UNUSED_PIN,
  UNUSED_PIN,
  false
};

// Mantiksal surus kanal indeksleri.
static constexpr uint8_t DRIVE_LEFT_FRONT_INDEX = 0;
static constexpr uint8_t DRIVE_LEFT_BOGIE_INDEX = 1;
static constexpr uint8_t DRIVE_RIGHT_FRONT_INDEX = 2;
static constexpr uint8_t DRIVE_RIGHT_BOGIE_INDEX = 3;

// Link / joystick haberlesmesi
// Joystick verisi PC'den USB seri portu uzerinden gelir.
static constexpr int8_t LINK_RX = UNUSED_PIN;
static constexpr int8_t LINK_TX = UNUSED_PIN;
static constexpr uint32_t LINK_BAUD = 115200;

// Guc izleme
// Harici batarya gerilim bolucu hatti henuz atanmadigi icin devre disidir.
static constexpr int8_t BATTERY_ADC = UNUSED_PIN;

// Varsayilan surus ayarlari
static constexpr int16_t MOTOR_PWM_MAX = 255;
static constexpr uint16_t JOYSTICK_TIMEOUT_MS = 500;
static constexpr uint8_t JOYSTICK_DEADZONE_PERCENT = 8;

}  // namespace Pins
