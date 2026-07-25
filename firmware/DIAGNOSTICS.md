# Ayrintili Diagnostics Kullanimi

Diagnostics testleri motor, servo ve lineer aktuatoru hareket ettirmez.

## Komutlar

- `DIAG,RUN`: Tum guvenli kontrolleri calistirir.
- `DIAG?`: Son testin ozetini yazar.
- `DIAG,LIVE`: Anlik PWM, joystick, E-stop ve hata durumunu yazar.
- `DIAG,PINS`: OLED, RFID, motor, aktuator, servo ve ADC pinlerini listeler.
- `DIAG,MODULES`: Her motor kanali ve servonun ayrintili durumunu yazar.

## Kontrol edilen alanlar

- RobotConfig ve Constants sayac uyumu
- Pin cakismalari ve atanmamis zorunlu pinler
- Kontrol dongusu, joystick timeout ve watchdog sureleri
- ESP32 bos heap ve minimum heap
- MotionController hedef/mevcut PWM ve E-stop
- Dort mantiksal BTS kanalinin durumu
- Lineer aktuator yonu ve kalan sure
- RFID firmware, son okuma durumu ve timeout sinirlari
- OLED I2C adresi ve ekran boyutu
- Batarya ADC ve gerilim degeri
- Servo hedef/mevcut acilari
- Joystick eksenleri, buton maskesi ve timeout
- ErrorManager aktif hata kodu

## Bilinen tasarim uyarilari

- Lineer aktuatorde limit switch veya pozisyon geri bildirimi bulunmuyor.
- Standart hobby servolarda gercek pozisyon geri bildirimi yoktur; raporlanan aci yazilimin tahminidir.
- `USE_POWER_MONITORING=true` iken `BATTERY_ADC=UNUSED_PIN` kalirsa test FAIL verir.
