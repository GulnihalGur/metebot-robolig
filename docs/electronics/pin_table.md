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
| Sol ön | Sol ön BTS | D0 | D1 | Ön teker bağımsız kontrol edilir. |
| Sol bogie | Sol orta + sol arka BTS | A2 | A3 | İki BTS'nin kontrol girişleri paraleldir; motor çıkışları ayrı kalır. |
| Sağ ön | Sağ ön BTS | D12 | D13 | Ön teker bağımsız kontrol edilir. |
| Sağ bogie | Sağ orta + sağ arka BTS | A5 | A6 | İki BTS'nin kontrol girişleri paraleldir; motor çıkışları ayrı kalır. |

Toplam fiziksel sürüş BTS sayısı **6**, mantıksal kontrol grubu sayısı **4**, kullanılan sürüş kontrol pini sayısı **8**'dir.

## Diğer pinler

| Alt sistem | Sinyal | Deneyap pini | Not |
|---|---|---|---|
| Lineer aktüatör BTS7 | RPWM | D4 | Aktüatör ileri yön PWM |
| Lineer aktüatör BTS7 | LPWM | A4 | Aktüatör geri yön PWM |
| Ana kıskaç servosu | PWM | D14 | Servo 0 |
| Ek servo 1 | PWM | MOSI / D7 | SPI kullanılmıyorsa servo pini |
| Ek servo 2 | PWM | MISO / D6 | SPI kullanılmıyorsa servo pini |
| Ek servo 3 | PWM | SCK / D5 | SPI kullanılmıyorsa servo pini |
| OLED | SDA | SDA / D10 | I2C veri |
| OLED | SCL | SCL / D11 | I2C saat |
| PN532 | PN532 TX → kart RX | A0 | `RFID_RX` |
| PN532 | PN532 RX ← kart TX | A1 | `RFID_TX` |
| Joystick bağlantısı | USB seri | Fiziksel pin yok | 115200 baud |
| Batarya ölçümü | ADC | Atanmadı | `UNUSED_PIN` |
| BTS8 | RPWM/LPWM | Atanmadı | Yedek sürücü |

## Bilerek kullanılmayan veya sakınılan pinler

| Pin | Neden |
|---|---|
| D2 / D3 | Programlama ve terminal UART hattı |
| D9 | Kart üzerindeki RGB LED ile ilişkili |
| D15 | GPKEY / manuel yükleme butonu ile ilişkili |
| D8 | SD kart hattı ile olası çakışmayı önlemek için ayrıldı |
| A7 | Gelecekte batarya ADC veya ek sensör için boş bırakıldı |
