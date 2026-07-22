# METEBOT Yazılım Mimarisi

## Katmanlar

```text
main.cpp
  ├─ MissionManager / StateMachine
  ├─ MotionController / RobotArm / SlotManager
  ├─ MotorDriver / ServoDriver / RFID / OLED / LinearActuator
  └─ UART / Joystick / FailSafe / Watchdog
```

## Sürüş mimarisi

Fiziksel sistemde altı motor ve altı BTS7960 bulunur. Yazılımda dört mantıksal sürüş kanalı vardır:

1. `left_front`
2. `left_bogie_shared` — sol orta ve sol arka BTS kontrol girişleri
3. `right_front`
4. `right_bogie_shared` — sağ orta ve sağ arka BTS kontrol girişleri

`MotorDriver` bu dört kanala PWM yazar. `MotionController` mevcut diferansiyel sürüş algoritmasını korur ve normal sürüşte aynı taraftaki ön ve bogie grubuna aynı PWM değerini gönderir.

```text
Joystick X/Y
   ↓
DifferentialOutput(left, right)
   ↓
MotorDriver::setDriveSpeeds(left, right)
   ├─ left_front  = left
   ├─ left_bogie  = left
   ├─ right_front = right
   └─ right_bogie = right
```

Gelecekte engel tırmanma kontrolü eklenirse `setDriveGroupSpeeds()` kullanılarak ön tekerlere bogie grubundan farklı PWM uygulanabilir. Bu yetenek sürüş algoritmasına otomatik olarak eklenmemiştir; yanlışlıkla sürekli farklı hız uygulanmasını önlemek için üst seviye karar mekanizması gerekir.

## Güvenlik davranışı

- Joystick zaman aşımında hareket hedefi sıfırlanır.
- `stopDrive()` dört mantıksal kanalı da durdurur.
- Acil durdurma sırasında RPWM ve LPWM değerleri sıfıra çekilir.
- ARM modunda sürüş motorları durdurulur.
- Başlatılamayan opsiyonel donanım için `Available` bayrakları kullanılır.

## Bağımlılık yönü

Üst seviye modüller alt seviye sürücülere bağımlıdır; donanım sürücüleri görev mantığına bağımlı olmamalıdır. Pin bilgileri yalnızca `Pins.h`, ayarlanabilir davranış sabitleri `RobotConfig.h` veya `AppConfig` içinde tutulur.
