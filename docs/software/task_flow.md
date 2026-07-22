# METEBOT Görev Akışı

## Üst seviye durumlar

```text
Açılış
  ↓
IDLE
  ↓ pilot komutu
MANUAL
  ↓ yük alma alanı
PICKUP
  ├─ ARM moduna geç
  ├─ yükü kavra
  ├─ RFID oku
  ├─ uygun slota yerleştir
  └─ slot kaydını güncelle
  ↓ beş slot dolu
DELIVERY
  ├─ hedef bölgeye sür
  ├─ doğru yükü seç
  └─ teslimatı kaydet
  ↓ teslimatlar tamam
ZIPLINE
  ├─ EXTENDING
  ├─ POSITIONING: manuel sürüş izinli
  ├─ SLIDING
  ├─ RETRACTING
  └─ COMPLETED
  ↓
FINISH
```

Her kritik hata durumundan `FAIL_SAFE` durumuna geçilebilir.

## Kontrol modları

- `DRIVE`: Joystick diferansiyel sürüşü kontrol eder; robot kolu pasiftir.
- `ARM`: Robot kolu kontrol edilir; sürüş motorları durur.
- Mod değişiminden sonra joystick eksenleri merkeze dönmeden hareket komutu kabul edilmez.

## Sürüş komutunun fiziksel dağılımı

Normal sürüşte sol ve sağ diferansiyel değerler dört mantıksal gruba dağıtılır:

```text
leftPwm  -> sol ön + sol orta/arka bogie
rightPwm -> sağ ön + sağ orta/arka bogie
```

Bu nedenle mevcut joystick davranışı değişmez. Donanım ayrımı yalnızca gelecekte ön teker ve bogie grubuna farklı komut verme olanağı sağlar.

## Fail-safe tetikleyicileri

- Aktif kontrol sırasında joystick timeout
- Düşük batarya (ADC hattı tamamlandıktan sonra)
- Motor/servo veya görev modülü kritik hatası
- Yazılımsal acil durdurma

Fail-safe sırasında motorlar, aktüatör ve güvenli olmayan servo hareketleri durdurulmalıdır.
