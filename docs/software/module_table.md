# METEBOT Modül Tablosu

| Modül | Sorumluluk | Temel bağımlılıklar |
|---|---|---|
| `main.cpp` | Kurulum, periyodik görevler, mod geçişleri | Tüm üst seviye servisler |
| `Joystick` | Seri joystick paketini ayrıştırma, deadzone, diferansiyel çıktı | `UARTLink`, `Constants` |
| `UARTLink` | Satır tabanlı seri haberleşme | Arduino `Stream` |
| `MotorDriver` | Dört mantıksal BTS kanalına güvenli PWM yazma | `Pins.h` |
| `MotionController` | Diferansiyel sürüş, PWM sınırı ve rampa | `MotorDriver`, `Joystick` |
| `ServoDriver` | Tekil servo donanım erişimi | `Pins.h` |
| `ServoManager` | Beş eksenin açı ve hız yönetimi | `ServoDriver` |
| `RobotArm` | Joystick eksenlerinden taban, omuz ve dirsek; hat switchten bilek; butonlardan kıskaç komutları | `ServoManager`, `Joystick` |
| `LinearActuator` | BTS7 üzerinden aktüatör ileri/geri/dur | `Pins::LINEAR_ACTUATOR_PINS` |
| `RFID` | PN532 UID okuma | UART, PN532 kütüphanesi |
| `SlotManager` | Beş slotun doluluk ve yük bilgisi | `Constants`, `Types` |
| `OLED` / `Display` | Kullanıcı durum ekranları | I2C |
| `PowerManager` | Batarya izleme | ADC pini atanırsa aktif |
| `StateMachine` | Robot görev durumunu saklama ve geçiş kontrolü | `Types` |
| `MissionManager` | PICKUP, DELIVERY, ZIPLINE ve FINISH akışı | Kontrol ve görev modülleri |
| `Zipline` | Aktüatör ve konumlandırma alt durumları | `MotionController`, `LinearActuator`, `FailSafe` |
| `Finish` | Görev sonu güvenli duruş ve raporlama | Motor, slot, OLED, aktüatör |
| `FailSafe` | Kritik hata durumunda güvenli duruş | Hareket sistemleri |
| `Watchdog` | Ana döngü kilitlenmesini algılama | ESP32 watchdog |
| `TaskScheduler` | Periyodik işlerin bloklamadan çalışması | Zaman tabanı |
| `ErrorManager` / `Diagnostics` | Hata kaydı ve tanılama | Seri çıktı |


## Kiskac surekli buton kontrolu

`GRIPPER_OPEN` ve `GRIPPER_CLOSE` butonlari sabit bir hedef aciya gitmez.
Buton basili tutuldugu surece `RobotArm::applyJoystick()` kiskac hedefini
`gripperControlSpeedDegPerSec` hiziyla kademeli olarak degistirir. Buton
birakildiginda hedef aci korunur. Iki buton ayni anda basilirsa hareket verilmez.
Minimum ve maksimum acilar `JointConfig` ve `ServoDriver` limitleriyle korunur.
