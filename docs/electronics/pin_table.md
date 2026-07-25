# METEBOT Pin Tablosu

## Kart ve temel kurallar

- Hedef kart: **Deneyap Kart 1A v2**.
- BTS7960 lojik girişleri 3.3 V kontrol sinyali alır; kart pinleri 5 V toleranslı değildir.
- Tüm kontrol kartları, BTS sürücüleri ve güç dönüştürücülerin GND hatları ortaklanmalıdır.
- Aynı pin adının iki farklı işlevde kullanılması yasaktır.
- `UNUSED_PIN (-1)` atanmış bir donanım yazılım tarafından başlatılmaz.

## Sürüş motorları — 8 kontrol pini

| Mantıksal grup | Fiziksel BTS | RPWM | LPWM | Açıklama |
|---|---:|---|---|---|
| Sol ön | Sol ön BTS | A1 | A2 | Ön teker bağımsız kontrol edilir. |
| Sol bogie | Sol orta + sol arka BTS | A3 | A4 | İki BTS'nin kontrol girişleri paraleldir; motor çıkışları ayrı kalır. |
| Sağ ön | Sağ ön BTS | A5 | A6 | Ön teker bağımsız kontrol edilir. |
| Sağ bogie | Sağ orta + sağ arka BTS | A7 | D14 | İki BTS'nin kontrol girişleri paraleldir; motor çıkışları ayrı kalır. |

Toplam fiziksel sürüş BTS sayısı **6**, mantıksal kontrol grubu sayısı **4**, kullanılan sürüş kontrol pini sayısı **8**'dir.

## Diğer pinler

| Alt sistem | Sinyal | Deneyap pini | Not |
|---|---|---|---|
| Lineer aktüatör BTS7 | RPWM | D0 | Aktüatör ileri yön PWM |
| Lineer aktüatör BTS7 | LPWM | D1 | Aktüatör geri yön PWM |
| Kol taban servosu | PWM | D4 | Servo 0 / `BASE` |
| Omuz servosu | PWM | MOSI / D7 | Servo 1 / `SHOULDER`; SPI kullanılmaz |
| Dirsek servosu | PWM | MISO / D6 | Servo 2 / `ELBOW`; SPI kullanılmaz |
| Bilek servosu | PWM | D12 | Servo 3 / `WRIST` |
| Kıskaç servosu | PWM | SCK / D5 | Servo 4 / `GRIPPER`; SPI kullanılmaz |
| OLED | SDA | SDA / D10 | I2C veri |
| OLED | SCL | SCL / D11 | I2C saat |
| PN532 | PN532 TX → kart RX | RX / D3 | `RFID_RX` |
| PN532 | PN532 RX ← kart TX | TX / D2 | `RFID_TX` |
| Joystick bağlantısı | Wi-Fi / UDP | Fiziksel pin yok | PC, kartın `METEBOT` ağına bağlanır; UDP 4210 |
| Batarya ölçümü | ADC | A0 | Batarya ADC pini |
| BTS8 | RPWM/LPWM | Atanmadı | Yedek sürücü |

## Bilerek kullanılmayan veya sakınılan pinler

| Pin | Neden |
|---|---|
| D9 | Kart üzerindeki RGB LED ile ilişkili |
| D15 | GPKEY / manuel yükleme butonu ile ilişkili |
| D8 | SD kart hattı ile olası çakışmayı önlemek için ayrıldı |
