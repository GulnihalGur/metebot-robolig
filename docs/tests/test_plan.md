# METEBOT Yazılım ve Donanım Test Planı

## 1. Doküman bilgileri

| Alan             | Değer                                   |
| ---------------- | --------------------------------------- |
| Proje            | METEBOT                                 |
| Takım            | ALTAY                                   |
| Yarışma          | TEKNOFEST RoboLig 2026                  |
| Doküman          | Yazılım ve Donanım Test Planı           |
| Hedef kart       | Deneyap Kart 1A v2 / ESP32              |
| Yazılım ortamı   | PlatformIO + Arduino Framework          |
| Son güncelleme   | YYYY-MM-DD                              |
| Hazırlayan       | Gül Nihal Gür / Yazılım Ekibi           |
| Test sorumluları | Yazılım, elektronik ve mekanik ekipleri |

---

## 2. Amaç

Bu dokümanın amacı, METEBOT robotunun yazılım ve donanım bileşenlerinin güvenli, tekrarlanabilir ve ölçülebilir şekilde test edilmesini sağlamaktır.

Testler aşağıdaki alanları kapsar:

* Projenin derlenmesi
* Sistem başlangıcı
* UART haberleşmesi
* Joystick veri işleme
* DRIVE ve ARM modları
* Altı motorlu sürüş sistemi
* Robot kolu ve servo sistemi
* RFID okuyucu
* Beş bölmeli slot sistemi
* OLED ekran
* Batarya ölçümü
* Lineer aktüatör
* Zipline görevi
* Görev durumları
* FailSafe ve hata kurtarma
* Finish durumu
* Tam sistem ve saha testi

Bu doküman testlerin nasıl uygulanacağını tarif eder. Testleri çalıştıran yazılım fonksiyonları gerektiğinde `Diagnostics.h` ve `Diagnostics.cpp` içerisinde geliştirilecektir.

---

## 3. Test sonucu sınıfları

Her test aşağıdaki sonuçlardan biriyle işaretlenmelidir:

| Sonuç   | Açıklama                                                                  |
| ------- | ------------------------------------------------------------------------- |
| PASS    | Test başarı kriterlerinin tamamı sağlandı.                                |
| FAIL    | En az bir başarı kriteri sağlanmadı.                                      |
| BLOCKED | Donanım, pin, yazılım veya mekanik eksikliği nedeniyle test uygulanamadı. |
| NOT RUN | Test henüz uygulanmadı.                                                   |
| PARTIAL | Testin yalnızca belirli bölümleri uygulanabildi.                          |

Bir test yalnızca gözleme dayanarak PASS yapılmamalıdır. Testin bütün başarı kriterleri doğrulanmalıdır.

---

## 4. Test kayıt formatı

Her test uygulamasından sonra aşağıdaki kayıt doldurulmalıdır:

| Alan                    | Değer                                     |
| ----------------------- | ----------------------------------------- |
| Test kimliği            |                                           |
| Tarih ve saat           |                                           |
| Testi uygulayan         |                                           |
| Yazılım commit numarası |                                           |
| Kullanılan kart         |                                           |
| Batarya gerilimi        |                                           |
| Test sonucu             | PASS / FAIL / BLOCKED / NOT RUN / PARTIAL |
| Gözlem                  |                                           |
| Hata açıklaması         |                                           |
| Açılan GitHub issue     |                                           |
| Tekrar test tarihi      |                                           |

Test sırasında kullanılan yazılımın Git commit numarası mutlaka kaydedilmelidir. Commitlenmemiş yerel değişiklikler bulunan kodla alınan sonuçlar resmî test sonucu kabul edilmemelidir.

---

# 5. Genel güvenlik kuralları

## 5.1 Test öncesi zorunlu kontroller

Her test oturumundan önce:

* Robotun mekanik bağlantıları kontrol edilmelidir.
* Açıkta iletken kablo bulunmamalıdır.
* BTS7960 sürücülerinin güç ve kontrol bağlantıları kontrol edilmelidir.
* Servo bağlantılarının yönü doğrulanmalıdır.
* Bataryada şişme, hasar veya aşırı ısınma olmamalıdır.
* Motor testlerinde robotun kaçmasını önleyecek fiziksel tedbir alınmalıdır.
* İlk motor testleri tekerlekler yerden kaldırılarak yapılmalıdır.
* İlk servo testlerinde servo kolları mümkünse mekanik yükten ayrılmalıdır.
* Lineer aktüatör hareket alanında insan eli veya mekanik engel bulunmamalıdır.
* Acil güç kesme yöntemi test başlamadan belirlenmelidir.
* Robotun güç sistemi ile USB bağlantısı arasında ortak topraklama gereksinimi kontrol edilmelidir.

## 5.2 Test sırası

Testler aşağıdaki sıra bozulmadan uygulanmalıdır:

1. Statik kod ve yapı kontrolü
2. Derleme testi
3. Güç verilmeden bağlantı kontrolü
4. USB ile başlangıç testi
5. Düşük güçlü donanım testleri
6. Servo testleri
7. Tekerlekler havadayken motor testleri
8. Yerde düşük hızlı sürüş testi
9. Görev modülü testleri
10. FailSafe testleri
11. Tam sistem testi
12. Saha testi

Bir önceki aşamadaki kritik testler PASS olmadan sonraki aşamaya geçilmemelidir.

---

# 6. Test ortamı

## 6.1 Gerekli ekipman

* Deneyap Kart 1A v2
* Veri aktarımını destekleyen USB kablosu
* PlatformIO kurulu bilgisayar
* Seri port terminali
* 3S LiPo batarya
* Uygun LiPo şarj cihazı
* Multimetre
* Altı DC motor
* Altı BTS7960 motor sürücüsü
* Robot kolu servoları
* PN532 RFID okuyucu
* OLED ekran
* RFID etiketleri
* Lineer aktüatör
* Yedinci BTS7960 sürücüsü
* Joystick veya joystick verisi gönderen test yazılımı
* Kronometre
* İzolasyon malzemeleri
* Gerekirse haricî servo güç kaynağı

## 6.2 Yazılım ayarları

PlatformIO ortamı:

```ini
[env:deneyap_esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
```

Seri haberleşme hızı:

```text
115200 baud
```

Joystick timeout süresi:

```text
500 ms
```

Joystick deadzone:

```text
%8
```

Düşük batarya eşiği:

```text
10.5 V
```

Batarya tam dolu kabulü:

```text
12.6 V
```

---

# 7. Testleri engelleyen mevcut konular

Aşağıdaki konular çözülmeden ilgili testler PASS yapılamaz:

## 7.1 Batarya ADC pini

`Pins::BATTERY_ADC` değeri henüz atanmamıştır:

Bu nedenle gerçek batarya ölçümü ve düşük batarya FailSafe testi şu an BLOCKED durumundadır.

---

## TP-STATIC-002 — Git çalışma alanı temizliği

### Amaç

Test edilen sürümün tekrar üretilebilir olmasını sağlamak.

### Adımlar

Terminalde çalıştır:

```bash
git status
```

Commit numarasını al:

```bash
git rev-parse --short HEAD
```

### Başarı kriterleri

* Test öncesinde beklenmeyen `modified`, `untracked` veya `deleted` dosya bulunmamalıdır.
* Test raporunda commit numarası kayıtlı olmalıdır.
* Test edilen kaynak kod GitHub’daki sürümle ilişkilendirilebilmelidir.

### Başarısızlık durumu

Commitlenmemiş değişiklik varsa test sonucu resmî PASS olarak kaydedilmemelidir.

---

## TP-STATIC-003 — Pin çakışması kontrolü

### Amaç

Aynı GPIO’nun birden fazla aktif donanım tarafından kullanılmadığını doğrulamak.

### Adımlar

1. `Pins.h` dosyasını aç.
2. OLED pinlerini listele.
3. RFID pinlerini listele.
4. Servo pinlerini listele.
5. Altı motor sürücüsünün RPWM ve LPWM pinlerini listele.
6. Lineer aktüatör pinlerini listele.
7. Batarya ADC pinini listele.
8. Aynı pinin birden fazla aktif işlevde kullanılıp kullanılmadığını kontrol et.
9. Pinleri Deneyap Kart 1A v2 şemasıyla karşılaştır.

### Başarı kriterleri

* Aktif donanımlar arasında pin çakışması bulunmamalıdır.
* Kullanılan pinlerin tamamı kart üzerinde erişilebilir olmalıdır.
* Girişe özel pin, çıkış gerektiren donanıma verilmemelidir.
* UART, I2C ve PWM görevleri kart özellikleriyle uyumlu olmalıdır.

---

# 9. Derleme ve yükleme testleri

## TP-BUILD-001 — Temiz derleme

### Amaç

Projenin sıfırdan hatasız derlenmesini doğrulamak.

### Adımlar

VS Code üzerinden:

```text
PlatformIO: Clean
PlatformIO: Build
```

veya terminalden:

```bash
pio run --target clean
pio run
```

### Başarı kriterleri

* Derleme hata vermemelidir.
* Eksik header hatası olmamalıdır.
* Tanımsız sembol hatası olmamalıdır.
* Kütüphane bağımlılık hatası olmamalıdır.
* Pin veya yapılandırma nedeniyle derleme durmamalıdır.
* Kritik uyarılar incelenmeden test PASS yapılmamalıdır.

---

## TP-BUILD-002 — Karta yazılım yükleme

### Amaç

Derlenen yazılımın Deneyap karta yüklenebildiğini doğrulamak.

### Adımlar

1. Kartı USB ile bilgisayara bağla.
2. Doğru seri portu seç.
3. PlatformIO Upload komutunu çalıştır.
4. Yükleme tamamlandıktan sonra Serial Monitor aç.
5. Baud değerini `115200` yap.

### Başarı kriterleri

* Kart doğru algılanmalıdır.
* Yazılım karta hatasız yüklenmelidir.
* Yükleme sonrasında kart yeniden başlamalıdır.
* Seri monitörde başlangıç mesajları görünmelidir.

---

# 10. Sistem başlangıç testleri

## TP-BOOT-001 — Başlangıç mesajları

### Amaç

Sistemin açılış sırasında temel bilgileri ve modül durumlarını doğru raporladığını doğrulamak.

### Adımlar

1. Robot hareket edemeyecek güvenli konumdayken kartı aç.
2. Serial Monitor’ü `115200 baud` ile aç.
3. Başlangıç mesajlarını kaydet.
4. Proje adı, sürüm ve modül başlangıç sonuçlarını kontrol et.

### Beklenen sonuç

Seri portta en az aşağıdaki türde bilgiler görülmelidir:

```text
METEBOT
TEKNOFEST Robolig 2026
Software Version: ...
Build Type: ...
MotionController started.
SlotManager started.
Finish module started.
```

Opsiyonel modüller hazır değilse ilgili uyarılar açıkça görünmelidir.

### Başarı kriterleri

* Sistem reset döngüsüne girmemelidir.
* Kritik modüller başlatılmalıdır.
* Başlatılamayan modüller için anlaşılır uyarı verilmelidir.
* Sistem başlangıçta kontrolsüz motor veya servo hareketi oluşturmamalıdır.
* Sistem başlangıç durumu `IDLE` olmalıdır.
* Kontrol modu `DRIVE` olmalıdır.

---

## TP-BOOT-002 — Güç verildiğinde motorların durması

### Amaç

Robot açılırken sürüş motorlarının kendiliğinden çalışmadığını doğrulamak.

### Hazırlık

* Tekerlekleri yerden kaldır.
* Motor güç bağlantılarını son kez kontrol et.
* Joystick merkezde olsun.

### Adımlar

1. Robotun ana gücünü aç.
2. On saniye boyunca altı motoru gözlemle.
3. Seri porttan komut gönderme.
4. Joystick paketi gönderme.

### Başarı kriterleri

* Altı motorun tamamı hareketsiz kalmalıdır.
* Motorlarda ani sıçrama olmamalıdır.
* BTS7960 sürücülerinde beklenmeyen ısınma olmamalıdır.

---

## TP-BOOT-003 — Servo başlangıç hareketi

### Amaç

Servo sisteminin başlangıçta güvenli biçimde home konumuna gittiğini doğrulamak.

### Hazırlık

İlk testte servo kollarını mekanik yükten ayır.

### Adımlar

1. Sisteme güç ver.
2. Dört servonun başlangıç davranışını gözlemle.
3. Servoların hedef home açılarına doğru hareketini kontrol et.
4. Mekanik sınır, sıkışma ve titreme olup olmadığını izle.

### Başarı kriterleri

* Servolar tanımlanan açı sınırlarını aşmamalıdır.
* Servo sürekli titrememelidir.
* Servo mekanik sınıra zorlanmamalıdır.
* Aşırı akım veya ısınma olmamalıdır.
* Home hareketi mekanik çarpışma oluşturmamalıdır.

### Not

Mevcut kod başlangıç sırasında `robotArm.moveHome()` çağırmaktadır. Fiziksel kol takılmadan önce home açıları mekanik ekiple doğrulanmalıdır.

---

# 11. UART haberleşme testleri

## TP-UART-001 — Komut listesinin görüntülenmesi

### Amaç

Kartın desteklediği komutların başlangıçta görüntülendiğini doğrulamak.

### Beklenen komutlar

```text
JOY,x,y,buttons
JOY,x,y,twist,throttle,hat,buttons
MODE,DRIVE
MODE,ARM
MODE?
STATE,IDLE
STATE,MANUAL
STATE,PICKUP
STATE?
ERROR?
POWER?
RECOVER
ZIP,EXTENDED
ZIP,POSITIONED
ZIP,SLID
ZIP,RETRACTED
```

### Başarı kriterleri

* Komut listesi okunabilir olmalıdır.
* Komut formatları kaynak kod ile uyuşmalıdır.

---

## TP-UART-002 — Durum sorgulama

### Adımlar

Seri porttan gönder:

```text
STATE?
```

### Beklenen cevap

```text
STATE,<MEVCUT_DURUM>
```

Örnek:

```text
STATE,IDLE
```

### Başarı kriterleri

* Sistem geçerli bir `RobotState` değeri döndürmelidir.
* Cevap boş veya bozuk olmamalıdır.

---

## TP-UART-003 — Hata sorgulama

### Adımlar

Gönder:

```text
ERROR?
```

### Beklenen cevap

Normal durumda:

```text
ERROR,NONE
```

### Başarı kriterleri

* Aktif hata doğru raporlanmalıdır.
* FailSafe sonrası hata kodu korunmalıdır.
* Recovery sonrası hata uygun şekilde temizlenmelidir.

---

## TP-UART-004 — Kontrol modu sorgulama

### Adımlar

Gönder:

```text
MODE?
```

### Beklenen cevap

```text
MODE,DRIVE
```

veya:

```text
MODE,ARM
```

### Başarı kriterleri

* Sistem gerçek kontrol modunu doğru raporlamalıdır.

---

## TP-UART-005 — Bilinmeyen komut

### Adımlar

Gönder:

```text
INVALID_COMMAND
```

### Beklenen sonuç

Sistem bilinmeyen komutu hata olarak raporlamalıdır.

Örnek cevap:

```text
ERROR,UNKNOWN_COMMAND,INVALID_COMMAND
```

### Başarı kriterleri

* Sistem kilitlenmemelidir.
* Motor veya servo hareket etmemelidir.
* Sonraki geçerli komutlar işlenmeye devam etmelidir.

---

# 12. Joystick veri işleme testleri

## TP-JOY-001 — Eski joystick paket formatı

### Amaç

Dört alanlı joystick paketinin işlenmesini doğrulamak.

### Adımlar

Gönder:

```text
JOY,0,0,0
```

Ardından:

```text
JOY,50,50,0
```

### Başarı kriterleri

* İlk paket geçerli kabul edilmelidir.
* Merkez paketinde motorlar hareket etmemelidir.
* İkinci pakette sistem geçerli x ve y değerlerini işlemelidir.
* Eski formatta throttle değeri otomatik olarak `%100` kabul edilmelidir.

---

## TP-JOY-002 — Yeni joystick paket formatı

### Paket biçimi

```text
JOY,x,y,twist,throttle,hat,buttons
```

### Adımlar

Gönder:

```text
JOY,0,0,0,20,-1,0
```

Ardından:

```text
JOY,0,50,0,20,-1,0
```

### Başarı kriterleri

* Paket yedi alanlı formatta kabul edilmelidir.
* `hat=-1` değeri kullanılmıyor olarak kabul edilmelidir.
* Throttle değeri motor hız sınırını etkilemelidir.
* Paket geçerli olarak kaydedilmelidir.

---

## TP-JOY-003 — Ham 0–1023 eksen değerleri

### Adımlar

Gönder:

```text
JOY,512,512,512,512,-1,0
```

### Başarı kriterleri

* X, Y ve twist değerleri merkeze yakın değere dönüştürülmelidir.
* Deadzone nedeniyle merkezde hareket oluşmamalıdır.
* Throttle yaklaşık orta seviyeye dönüştürülmelidir.

---

## TP-JOY-004 — Deadzone testi

### Adımlar

Sırayla gönder:

```text
JOY,0,5,0
JOY,0,8,0
JOY,0,9,0
JOY,0,-8,0
JOY,0,-9,0
```

### Başarı kriterleri

* Mutlak değeri `%8` veya daha düşük eksen değerleri sıfır kabul edilmelidir.
* `%9` değerinde hareket komutu oluşabilmelidir.
* Merkez titreşimleri motor hareketine dönüşmemelidir.

---

## TP-JOY-005 — Geçersiz paketler

### Adımlar

Sırayla gönder:

```text
JOY
JOY,1
JOY,1,2
JOY,1,2,3,4
JOY,1,2,3,4,5
JOY,1,2,3,4,5,6,7
```

### Başarı kriterleri

* Yalnızca dört veya yedi alanlı geçerli format kabul edilmelidir.
* Geçersiz paket robotu hareket ettirmemelidir.
* Sistem kilitlenmemelidir.
* Sonraki geçerli paket işlenebilmelidir.

---

# 13. DRIVE ve ARM modu testleri

## TP-MODE-001 — DRIVE moduna geçiş

### Adımlar

Gönder:

```text
MODE,DRIVE
```

Ardından:

```text
MODE?
```

### Beklenen sonuç

```text
MODE,DRIVE
```

### Başarı kriterleri

* Robot kolu joystick kontrolüne kapatılmalıdır.
* Sürüş kontrolü etkinleşmelidir.
* Mod geçişinden sonra joystick merkezlenmeden hareket başlamamalıdır.

---

## TP-MODE-002 — ARM moduna geçiş

### Ön koşul

Servo sistemi başarıyla başlatılmış olmalıdır.

### Adımlar

Gönder:

```text
MODE,ARM
```

Ardından:

```text
MODE?
```

### Beklenen cevap türü

```text
OK,MODE,ARM,CENTER_JOYSTICK
```

ve sorgu sonrasında:

```text
MODE,ARM
```

### Başarı kriterleri

* Sürüş motorları durmalıdır.
* Robot kolu kontrolü etkinleşmelidir.
* Joystick merkeze dönmeden kol hareket etmemelidir.
* Servo sistemi hazır değilse sistem `ARM_NOT_AVAILABLE` hatası vermelidir.

---

## TP-MODE-003 — Mod butonu

### Bilgi

Buton indeksleri sıfır tabanlıdır.

```text
Gripper open: buton 0
Gripper close: buton 1
Mode toggle: buton 2
Zipline confirm: buton 3
Wrist kontrolu: hat switch yukari/asagi
```

Buton 2’nin bit maskesi:

```text
1 << 2 = 4
```

### Adımlar

Önce buton bırakılmış paket gönder:

```text
JOY,0,0,0,100,-1,0
```

Sonra buton 2 basılı paket gönder:

```text
JOY,0,0,0,100,-1,4
```

Sonra tekrar bırak:

```text
JOY,0,0,0,100,-1,0
```

### Başarı kriterleri

* Butona ilk basışta mod yalnızca bir kez değişmelidir.
* Buton basılı tutulduğunda sürekli mod değiştirmemelidir.
* Bir sonraki mod değişimi için buton bırakılıp yeniden basılmalıdır.
* Mod geçişinde sürüş ve kol güvenli biçimde durdurulmalıdır.

---

# 14. Sürüş sistemi testleri

## TP-DRIVE-001 — Altı motorun tek tek bağlantı testi

### Amaç

Her motorun doğru BTS7960 kanalına bağlı olduğunu doğrulamak.

### Hazırlık

* Robot tekerlekleri yerden kaldırılmalıdır.
* Test mümkünse `Diagnostics` içindeki motor test fonksiyonuyla yapılmalıdır.
* Her seferinde yalnızca bir motor çalıştırılmalıdır.
* Düşük PWM kullanılmalıdır.

### Kontrol edilecek motorlar

| İndeks | Motor    |
| ------ | -------- |
| 0      | Sol ön   |
| 1      | Sol orta |
| 2      | Sol arka |
| 3      | Sağ ön   |
| 4      | Sağ orta |
| 5      | Sağ arka |

### Adımlar

Her motor için:

1. Motoru düşük hızda ileri çalıştır.
2. Bir saniye gözlemle.
3. Motoru durdur.
4. Düşük hızda geri çalıştır.
5. Bir saniye gözlemle.
6. Motoru durdur.

### Başarı kriterleri

* Yalnızca seçilen motor hareket etmelidir.
* Motor ileri ve geri dönebilmelidir.
* Dur komutunda tamamen durmalıdır.
* BTS7960 aşırı ısınmamalıdır.
* Kablo veya konektör ısınmamalıdır.
* Motor yönü pin tablosundaki tanımla uyuşmalıdır.

---

## TP-DRIVE-002 — İleri sürüş

### Hazırlık

İlk uygulama tekerlekler havadayken yapılmalıdır.

### Adımlar

1. `MANUAL` durumuna geç:

```text
STATE,MANUAL
```

2. DRIVE moduna geç:

```text
MODE,DRIVE
```

3. Merkez paket gönder:

```text
JOY,0,0,0,20,-1,0
```

4. İleri paket gönder:

```text
JOY,0,50,0,20,-1,0
```

5. Merkez paket gönder:

```text
JOY,0,0,0,20,-1,0
```

### Başarı kriterleri

* Sol taraftaki üç motor aynı yönde dönmelidir.
* Sağ taraftaki üç motor aynı yönde dönmelidir.
* Robot ileri gitmeye karşılık gelen yönde hareket etmelidir.
* Merkez paketinden sonra motorlar durmalıdır.
* Hız değişimi rampalı ve kontrol edilebilir olmalıdır.

---

## TP-DRIVE-003 — Geri sürüş

### Adımlar

Gönder:

```text
JOY,0,-50,0,20,-1,0
```

Sonra:

```text
JOY,0,0,0,20,-1,0
```

### Başarı kriterleri

* Altı motor ileri sürüşün ters yönünde dönmelidir.
* Robot düz geri hareket etmelidir.
* Sağ veya sola belirgin sapma varsa mekanik ve motor yönleri kontrol edilmelidir.

---

## TP-DRIVE-004 — Yerinde sağ ve sol dönüş

### Sağa dönüş

```text
JOY,50,0,0,20,-1,0
```

### Sola dönüş

```text
JOY,-50,0,0,20,-1,0
```

### Başarı kriterleri

* Sağ ve sol motor grupları uygun zıt yönlerde çalışmalıdır.
* Robot kendi merkezi çevresinde kontrollü dönmelidir.
* Dönüş yönü joystick X ekseniyle uyumlu olmalıdır.

---

## TP-DRIVE-005 — Çapraz sürüş komutu

### Adımlar

Gönder:

```text
JOY,50,50,0,20,-1,0
```

Ardından:

```text
JOY,-50,50,0,20,-1,0
```

### Başarı kriterleri

* Sol ve sağ PWM oranları korunmalıdır.
* PWM değeri izin verilen maksimum değeri aşmamalıdır.
* Robot kavisli hareket etmelidir.
* Doygunluk durumunda yön oranı bozulmamalıdır.

---

## TP-DRIVE-006 — Throttle hız sınırı

### Adımlar

Aynı Y ekseni değeriyle farklı throttle değerleri gönder:

```text
JOY,0,60,0,10,-1,0
JOY,0,60,0,30,-1,0
JOY,0,60,0,60,-1,0
JOY,0,60,0,100,-1,0
```

Her komut arasında merkez paketi gönder.

### Başarı kriterleri

* Throttle yükseldikçe izin verilen motor hızı artmalıdır.
* Throttle düşükken robot aniden tam hıza çıkmamalıdır.
* Maksimum PWM yapılandırılan sürüş sınırını aşmamalıdır.

---

## TP-DRIVE-007 — Ani yön değişimi

### Adımlar

1. Orta hızda ileri komut gönder.
2. Kısa süre sonra geri komut gönder.
3. Motor davranışını gözlemle.

### Başarı kriterleri

* Motor PWM değeri rampalı şekilde değişmelidir.
* Motor doğrudan tam ileri hızdan tam geri hıza atlamamalıdır.
* Güç hattında reset oluşmamalıdır.
* BTS7960 aşırı ısınmamalıdır.

---

## TP-DRIVE-008 — Stop davranışı

### Adımlar

1. Robotu düşük hızda hareket ettir.
2. Merkez joystick paketi gönder.
3. Ardından motor güncellemelerini gözlemle.
4. FailSafe tetiklendiğinde davranışı ayrıca kontrol et.

### Başarı kriterleri

* Normal merkezlemede robot kontrollü biçimde durmalıdır.
* FailSafe durumunda motorlar beklemeden durmalıdır.
* Hedef ve mevcut PWM değerleri sıfıra dönmelidir.

---

# 15. Robot kolu ve servo testleri

## TP-ARM-001 — Servoların tek tek doğrulanması

### Hazırlık

* Servo mekanik kolları ilk testte ayrılmalıdır.
* Haricî servo beslemesi kullanılıyorsa topraklar ortaklanmalıdır.
* Servo gerilimi üretici sınırlarına uygun olmalıdır.

### Kontrol edilecek eklemler

| Eklem  | ServoJoint |
| ------ | ---------- |
| Taban  | BASE       |
| Omuz   | SHOULDER   |
| Dirsek | ELBOW      |
| Kıskaç | GRIPPER    |

### Adımlar

Her servo için:

1. Minimum güvenli açıya git.
2. Home açısına git.
3. Maksimum güvenli açıya git.
4. Home açısına geri dön.

### Başarı kriterleri

* Doğru servo hareket etmelidir.
* Servo tanımlı minimum ve maksimum açı dışına çıkmamalıdır.
* Hareket sırasında titreme olmamalıdır.
* Servo mekanik sınıra zorlanmamalıdır.
* Diğer servolar istemeden hareket etmemelidir.

---

## TP-ARM-002 — ARM modunda sürüşün kilitlenmesi

### Adımlar

1. `MANUAL` durumuna geç.
2. `MODE,ARM` komutu gönder.
3. Joystick merkezle.
4. İleri sürüşe karşılık gelen Y ekseni komutunu gönder.

### Başarı kriterleri

* Sürüş motorları hareket etmemelidir.
* Joystick verisi yalnızca robot koluna uygulanmalıdır.
* Aynı anda hem motor hem servo hareketi oluşmamalıdır.

---

## TP-ARM-003 — DRIVE modunda kolun kilitlenmesi

### Adımlar

1. `MODE,DRIVE` komutu gönder.
2. Robot kolunu hareket ettirecek joystick komutları gönder.
3. Kıskaç butonlarını dene.

### Başarı kriterleri

* Robot kolu joystick kontrolüyle hareket etmemelidir.
* DRIVE modunda sürüş kontrolü çalışmalıdır.
* Kontrol modları birbirinden kesin olarak ayrılmalıdır.

---

## TP-ARM-004 — Taban kontrolü

### Adımlar

1. ARM moduna geç.
2. Joysticki merkezle.
3. Tabanı kontrol eden ekseni düşük değerde hareket ettir.
4. Pozitif ve negatif yönleri dene.
5. Joysticki merkeze getir.

### Başarı kriterleri

* Yalnızca taban eklemi hareket etmelidir.
* Hareket yönü beklenen yönle uyumlu olmalıdır.
* Joystick merkezdeyken hedef açı değişmemelidir.
* Açı sınırları aşılmamalıdır.

---

## TP-ARM-005 — Omuz ve dirsek kontrolü

### Adımlar

Omuz ve dirsek için kullanılan joystick eksenlerini ayrı ayrı pozitif ve negatif yönde test et.

### Başarı kriterleri

* Her eksen yalnızca ilgili eklemi kontrol etmelidir.
* Hareketler birbirine karışmamalıdır.
* Eklemler mekanik sınırlarını aşmamalıdır.
* Düşük joystick sapmasında kontrol edilemeyecek hızlanma olmamalıdır.

---

## TP-ARM-006 — Kıskaç açma

### Bilgi

Kıskaç açma buton indeksi:

```text
0
```

Bit maskesi:

```text
1
```

### Adımlar

Önce bırakılmış paket gönder:

```text
JOY,0,0,0,30,-1,0
```

Sonra açma butonunu gönder:

```text
JOY,0,0,0,30,-1,1
```

### Başarı kriterleri

* Kıskaç tanımlanan açık açıya gitmelidir.
* Kıskaç mekanik sınıra zorlanmamalıdır.
* Diğer eklemler hareket etmemelidir.

---

## TP-ARM-007 — Kıskaç kapatma

### Bilgi

Kıskaç kapatma buton indeksi:

```text
1
```

Bit maskesi:

```text
2
```

### Adımlar

Önce bırakılmış paket gönder:

```text
JOY,0,0,0,30,-1,0
```

Sonra kapatma butonunu gönder:

```text
JOY,0,0,0,30,-1,2
```

### Başarı kriterleri

* Kıskaç tanımlanan kapalı açıya gitmelidir.
* Nesne tutulduğunda servo sürekli zorlanmamalıdır.
* Diğer eklemler hareket etmemelidir.

---

## TP-ARM-008 — Kol haberleşme timeout’u

### Adımlar

1. ARM moduna geç.
2. Robot kolunu hareket ettiren geçerli paketler gönder.
3. Paket göndermeyi durdur.
4. En az bir saniye bekle.

### Başarı kriterleri

* Yeni hedef hareket üretilmemelidir.
* Robot kolu kontrolsüz biçimde hareket etmeye devam etmemelidir.
* Mevcut hedef güvenli şekilde tutulmalı veya hareket durdurulmalıdır.

---

# 16. RFID testleri

## TP-RFID-001 — RFID başlangıcı

### Adımlar

1. Sistemi aç.
2. Seri port başlangıç mesajlarını kontrol et.
3. RFID modülünün başlangıç sonucunu kaydet.

### Başarı kriterleri

Başarılı durumda:

```text
RFID started.
```

Başarısız durumda anlaşılır uyarı verilmelidir:

```text
WARNING: RFID could not start.
```

Sistem RFID hatası nedeniyle kontrolsüz hareket etmemelidir.

---

## TP-RFID-002 — Tek RFID etiketi okuma

### Hazırlık

`Diagnostics` veya görev test arayüzü üzerinden RFID okuma fonksiyonuna erişim sağlanmalıdır.

### Adımlar

1. Okuyucunun çevresindeki diğer RFID etiketlerini uzaklaştır.
2. Tek etiketi okuyucuya yaklaştır.
3. UID değerini kaydet.
4. Etiketi uzaklaştır.
5. Aynı etiketi tekrar okut.

### Başarı kriterleri

* UID boş olmamalıdır.
* Aynı etiket tekrar okutulduğunda aynı UID alınmalıdır.
* UID formatı tutarlı olmalıdır.
* Okuma sırasında sistem kilitlenmemelidir.

---

## TP-RFID-003 — Etiket yokken okuma

### Adımlar

1. RFID okuyucunun yakınında etiket olmadığından emin ol.
2. Okuma işlemi başlat.
3. Sistem davranışını gözlemle.

### Başarı kriterleri

* Rastgele UID üretilmemelidir.
* Slot kaydı oluşturulmamalıdır.
* Sistem kilitlenmemelidir.
* Başarısız okuma uygun şekilde raporlanmalıdır.

---

## TP-RFID-004 — Birden fazla etiket etkisi

### Adımlar

1. İki etiketi aynı anda okuyucuya yaklaştır.
2. Okuma sonucunu kaydet.
3. Etiketleri tek tek ayırarak tekrar dene.

### Başarı kriterleri

* Sistem kararsız veya rastgele slot kaydı oluşturmamalıdır.
* Kullanım prosedüründe aynı anda yalnızca bir etiket okutulması gerektiği belirtilmelidir.
* Yanlış UID kaydı oluşursa test FAIL olmalıdır.

---

# 17. SlotManager testleri

Bu testler bilgisayar üzerinde birim testiyle veya `Diagnostics` test fonksiyonlarıyla yapılabilir. Fiziksel donanım gerektirmez.

## TP-SLOT-001 — Başlangıç durumu

### Adımlar

1. SlotManager’ı beş slotla başlat.
2. `slotCount()` sonucunu kontrol et.
3. Dolu slot sayısını kontrol et.
4. Boş slot sayısını kontrol et.

### Başarı kriterleri

* Slot sayısı `5` olmalıdır.
* Başlangıçta dolu slot sayısı `0` olmalıdır.
* Başlangıçta boş slot sayısı `5` olmalıdır.
* Bütün slotlar boş olmalıdır.

---

## TP-SLOT-002 — İlk boş slota kayıt

### Adımlar

1. Örnek bir UID ve şehir oluştur.
2. Tercih edilen slot vermeden kayıt yap.
3. Kayıt yapılan slotu kontrol et.

### Başarı kriterleri

* Yük ilk boş slota yerleşmelidir.
* Slot `occupied=true` olmalıdır.
* UID doğru saklanmalıdır.
* Şehir doğru saklanmalıdır.
* Dolu slot sayısı bir artmalıdır.

---

## TP-SLOT-003 — Tercih edilen slota kayıt

### Adımlar

1. Boş olan belirli bir slot seç.
2. UID ve şehir bilgisiyle kayıt yap.
3. İlgili slotu kontrol et.

### Başarı kriterleri

* Kayıt belirtilen slota yapılmalıdır.
* Başka slot değişmemelidir.

---

## TP-SLOT-004 — Dolu slota ikinci kayıt

### Adımlar

1. Bir slotu doldur.
2. Aynı slota farklı UID kaydetmeye çalış.

### Başarı kriterleri

* İkinci kayıt reddedilmelidir.
* İlk kayıt bozulmamalıdır.
* Dolu slot sayısı değişmemelidir.

---

## TP-SLOT-005 — Aynı UID’nin tekrar eklenmesi

### Adımlar

1. Bir UID’yi ilk slota kaydet.
2. Aynı UID’yi başka slota kaydetmeye çalış.

### Başarı kriterleri

* Aynı UID’nin ikinci kez eklenmesi reddedilmelidir.
* Tek bir UID yalnızca bir slota bağlı olmalıdır.

---

## TP-SLOT-006 — Beş slotun doldurulması

### Adımlar

Beş farklı UID’yi beş farklı slota ekle.

### Başarı kriterleri

* Beş slotun tamamı dolu olmalıdır.
* Boş slot sayısı sıfır olmalıdır.
* `full()` sonucu true olmalıdır.
* Altıncı kayıt reddedilmelidir.

---

## TP-SLOT-007 — Slot silme

### Adımlar

1. Bir slotu doldur.
2. İlgili slot kaydını sil.
3. Slot durumunu kontrol et.

### Başarı kriterleri

* Slot boş duruma dönmelidir.
* UID ve şehir bilgisi temizlenmelidir.
* Dolu slot sayısı bir azalmalıdır.
* Silinen slot yeniden kullanılabilmelidir.

---

## TP-SLOT-008 — Geçersiz slot indeksi

### Adımlar

Aşağıdaki türde geçersiz indeksleri dene:

```text
5
6
255
```

### Başarı kriterleri

* Geçersiz slot erişimi reddedilmelidir.
* Bellek taşması veya sistem çökmesi olmamalıdır.
* Mevcut slot verileri değişmemelidir.

---

# 18. Pickup görevi testleri

## TP-PICKUP-001 — PICKUP durumuna geçiş

### Adımlar

1. Sistemin `IDLE` durumunda olduğunu doğrula.
2. `MANUAL` durumuna geç:

```text
STATE,MANUAL
```

3. PICKUP durumuna geç:

```text
STATE,PICKUP
```

4. Durumu sorgula:

```text
STATE?
```

### Başarı kriterleri

* Geçiş StateMachine kurallarına uygun olmalıdır.
* Son durum `PICKUP` olmalıdır.
* FailSafe aktifken geçiş reddedilmelidir.

---

## TP-PICKUP-002 — RFID ile yük kaydı

### Hazırlık

`MissionManager::scanPickupItem(city, preferredSlot)` fonksiyonuna Diagnostics veya geçici test harness üzerinden erişim sağlanmalıdır.

### Adımlar

1. PICKUP durumuna geç.
2. Bir RFID etiketi okut.
3. Geçerli şehir bilgisi ver.
4. Tercih edilen slot vermeden kayıt yap.
5. SlotManager verisini kontrol et.

### Başarı kriterleri

* RFID UID doğru okunmalıdır.
* Yük ilk boş slota kaydedilmelidir.
* Şehir bilgisi ilgili slota yazılmalıdır.
* Aynı UID ikinci kez kaydedilmemelidir.
* Okuma başarısızsa slot değişmemelidir.

---

## TP-PICKUP-003 — Beş yük tamamlandıktan sonra DELIVERY geçişi

### Adımlar

1. Beş farklı UID’yi slotlara kaydet.
2. `confirmPickupCompleted()` işlemini çalıştır.
3. Durumu sorgula.

### Başarı kriterleri

* Beş slot dolmadan pickup tamamlanmamalıdır.
* Beş slot dolduğunda sistem `DELIVERY` durumuna geçebilmelidir.
* Slot verileri geçiş sırasında korunmalıdır.

---

# 19. Delivery görevi testleri

## TP-DELIVERY-001 — Teslim edilecek yükün bulunması

### Adımlar

1. Slotlara önceden bilinen RFID kayıtları ekle.
2. DELIVERY durumuna geç.
3. Kayıtlı etiketlerden birini okut.
4. `scanDeliveryItem()` sonucunu kontrol et.

### Başarı kriterleri

* Doğru slot indeksi bulunmalıdır.
* Okuma yapılır yapılmaz slot kaydı silinmemelidir.
* Bilinmeyen UID için geçerli slot döndürülmemelidir.

---

## TP-DELIVERY-002 — Fiziksel bırakma sonrası slot temizleme

### Adımlar

1. Kayıtlı yükün slotunu belirle.
2. Pilot yükü fiziksel olarak bıraksın.
3. `confirmDeliveryItemReleased(slotIndex)` işlemini çalıştır.
4. Slot verisini kontrol et.

### Başarı kriterleri

* Yalnızca doğrulanan slot temizlenmelidir.
* Diğer slotlar korunmalıdır.
* Aynı slotun ikinci kez bırakılması reddedilmelidir.

---

## TP-DELIVERY-003 — Tüm teslimatlar sonrası ZIPLINE geçişi

### Adımlar

1. Beş slotun tamamını teslim edilmiş olarak temizle.
2. `confirmDeliveryCompleted()` işlemini çalıştır.
3. Durumu sorgula.

### Başarı kriterleri

* En az bir slot doluyken delivery tamamlanmamalıdır.
* Bütün slotlar boş olduğunda sistem ZIPLINE durumuna geçebilmelidir.
* Zipline alt durumu beklenen başlangıç durumunda olmalıdır.

---

# 20. OLED ekran testleri

## TP-OLED-001 — OLED başlangıcı

### Adımlar

1. Sistemi aç.
2. Başlangıç mesajlarını kontrol et.
3. OLED görüntüsünü gözlemle.

### Başarı kriterleri

* OLED başarıyla başlatılmalıdır.
* Başlangıç bilgisi okunabilir olmalıdır.
* Ekranda bozuk piksel dizisi veya sürekli titreşim olmamalıdır.
* OLED başlatılamazsa sistem uyarı vermeli, fakat kritik sürüş sistemi kontrolsüz davranmamalıdır.

---

## TP-OLED-002 — Durum gösterimi

### Adımlar

Sırasıyla aşağıdaki durumlara geç:

```text
IDLE
MANUAL
PICKUP
```

Kontrol modlarını da değiştir:

```text
DRIVE
ARM
```

### Başarı kriterleri

* OLED mevcut robot durumunu doğru göstermelidir.
* DRIVE ve ARM modu doğru gösterilmelidir.
* Metinler 128×64 ekranda taşmamalıdır.
* Görüntü anlaşılır olmalıdır.

---

## TP-OLED-003 — FailSafe gösterimi

### Adımlar

1. Haberleşme timeout’u ile FailSafe tetikle.
2. OLED ekranını gözlemle.

### Başarı kriterleri

* FailSafe durumu açıkça görünmelidir.
* Hata nedeni görüntülenmelidir.
* Normal görev ekranıyla karışmamalıdır.

---

# 21. Batarya ve güç testleri

## TP-POWER-001 — ADC pininin doğrulanması

### Mevcut durum

Bu test, `Pins::BATTERY_ADC` atanmadığı sürece BLOCKED durumundadır.

### Adımlar

1. Batarya voltaj bölücü devresini kontrol et.
2. Bölücü dirençlerini ölç.
3. ADC pinini doğrula.
4. Multimetreyle gerçek batarya voltajını ölç.
5. Seri porttan gönder:

```text
POWER?
```

### Başarı kriterleri

* ADC pinindeki voltaj kartın izin verilen giriş gerilimini aşmamalıdır.
* Yazılım ölçümü ile multimetre ölçümü arasındaki fark kabul edilen toleransta olmalıdır.
* Önerilen ilk tolerans: en fazla `±0.20 V`.
* Ölçüm sonucu `NaN`, negatif veya fiziksel olarak imkânsız olmamalıdır.

---

## TP-POWER-002 — Doluluk yüzdesi

### Adımlar

Farklı bilinen batarya voltajlarında `POWER?` komutunu çalıştır.

### Başarı kriterleri

* Yüzde değeri `0–100` aralığında olmalıdır.
* Voltaj yükseldikçe tahmini yüzde düşmemelidir.
* `12.6 V` civarında değer tam doluya yakın olmalıdır.
* Düşük voltajda değer sıfıra yaklaşmalıdır.

---

## TP-POWER-003 — Düşük batarya FailSafe

### Mevcut durum

ADC pini ve güvenli test güç kaynağı hazırlanmadan uygulanmamalıdır.

### Hazırlık

Gerçek LiPo’yu bilinçli şekilde tehlikeli seviyeye boşaltmak yerine ayarlanabilir laboratuvar güç kaynağı kullanılmalıdır.

### Adımlar

1. Sistemi nominal voltajda başlat.
2. Gerilimi kontrollü biçimde azalt.
3. `10.5 V` eşiği çevresindeki davranışı gözlemle.
4. Durumu ve hata kodunu sorgula.

### Başarı kriterleri

* Tek bir gürültülü örnek nedeniyle FailSafe tetiklenmemelidir.
* Yeterli ardışık düşük ölçüm sonrası FailSafe tetiklenmelidir.
* Motorlar durmalıdır.
* Robot kolu pasifleştirilmelidir.
* Hata `LOW_BATTERY` olarak kaydedilmelidir.
* Yeterli voltaj sağlanmadan `RECOVER` kabul edilmemelidir.

---

# 22. Haberleşme kaybı ve FailSafe testleri

## TP-SAFE-001 — Joystick timeout

### Adımlar

1. MANUAL ve DRIVE moduna geç.
2. Düzenli joystick paketleri gönder.
3. Robotu düşük hızda çalıştır.
4. Paket göndermeyi tamamen kes.
5. En az bir saniye gözlemle.

### Başarı kriterleri

* Son geçerli paketten yaklaşık `500 ms` sonra haberleşme kaybı algılanmalıdır.
* Motorlar durmalıdır.
* Robot kolu pasifleştirilmelidir.
* Lineer aktüatör durmalıdır.
* Sistem `FAIL_SAFE` durumuna geçmelidir.
* Hata `COMMUNICATION_LOST` olarak kaydedilmelidir.

---

## TP-SAFE-002 — FailSafe durumunda hareket engeli

### Adımlar

1. Haberleşme kaybıyla FailSafe oluştur.
2. Hareket joystick paketi gönder.
3. `MODE,ARM` gönder.
4. Durum değiştirme komutu gönder.

### Başarı kriterleri

* FailSafe çözülmeden hareket başlamamalıdır.
* Kontrol modu değişimi engellenmelidir.
* Normal görev durumu değişimi engellenmelidir.
* Sistem hata mesajı vermelidir.

---

## TP-SAFE-003 — Haberleşme sonrası recovery

### Adımlar

1. Haberleşme kaybı ile FailSafe oluştur.
2. Geçerli ve merkezlenmiş joystick paketi gönder:

```text
JOY,0,0,0,20,-1,0
```

3. Gönder:

```text
RECOVER
```

4. Durumu ve hatayı sorgula:

```text
STATE?
ERROR?
MODE?
```

### Başarı kriterleri

* Geçerli joystick bağlantısı dönmeden recovery reddedilmelidir.
* Bağlantı döndükten sonra recovery başarılı olmalıdır.
* Sistem FailSafe öncesindeki uygun görev durumuna dönebilmelidir.
* Hata temizlenmelidir.
* Kontrol modu güvenlik için DRIVE olmalıdır.
* Joystick merkezlenmeden hareket başlamamalıdır.
* Slot verileri silinmemelidir.

---

## TP-SAFE-004 — FailSafe sırasında slot verilerinin korunması

### Adımlar

1. Slotlara örnek yükler kaydet.
2. PICKUP veya DELIVERY sırasında FailSafe oluştur.
3. Recovery işlemini tamamla.
4. Slot kayıtlarını tekrar kontrol et.

### Başarı kriterleri

* FailSafe sırasında slot kayıtları silinmemelidir.
* Recovery sonrası görev verileri korunmalıdır.
* FailSafe öncesi görev durumu güvenli şekilde geri yüklenmelidir.

---

# 23. Lineer aktüatör testleri

## TP-ACT-001 — Aktüatör pin ve yön testi

### Mevcut durum

Lineer aktüatör pinleri atanmadığı için bu test şu an BLOCKED durumundadır.

### Hazırlık

* Aktüatör mekanik yükten ayrılmalıdır.
* Hareket mesafesi güvenli olmalıdır.
* Güç kesme yöntemi hazır olmalıdır.
* Aktüatörde dahili limit switch bulunup bulunmadığı doğrulanmalıdır.

### Adımlar

1. Düşük hızda kısa süreli uzatma komutu ver.
2. Aktüatörü durdur.
3. Düşük hızda kısa süreli geri çekme komutu ver.
4. Aktüatörü durdur.

### Başarı kriterleri

* `extend()` aktüatörü doğru yönde uzatmalıdır.
* `retract()` doğru yönde geri çekmelidir.
* `stop()` hareketi durdurmalıdır.
* İki PWM yönü aynı anda aktif olmamalıdır.
* Aktüatör mekanik sınıra zorlanmamalıdır.

---

## TP-ACT-002 — Süreli hareket

### Adımlar

1. Aktüatöre düşük hızla kısa bir `moveFor()` komutu ver.
2. Kronometre ile hareket süresini ölç.
3. `update()` çağrılarının devam ettiğini doğrula.

### Başarı kriterleri

* Aktüatör belirtilen süre sonunda otomatik durmalıdır.
* Ana döngü bloke olmamalıdır.
* Süre dolduktan sonra yön `Stop` olmalıdır.
* `remainingMs()` değeri mantıklı şekilde azalmalıdır.

---

## TP-ACT-003 — FailSafe sırasında durma

### Adımlar

1. Aktüatörü hareket ettir.
2. FailSafe tetikle.
3. Aktüatörü gözlemle.

### Başarı kriterleri

* Aktüatör gecikmeden durmalıdır.
* Recovery sonrasında kendiliğinden eski harekete devam etmemelidir.
* Yeni pilot onayı olmadan hareket başlamamalıdır.

---

# 24. Zipline görev testleri

## 24.1 Beklenen zipline alt durumları

```text
IDLE
EXTENDING
POSITIONING
SLIDING
RETRACTING
COMPLETED
CANCELLED
FAILED
```

## TP-ZIP-001 — ZIPLINE başlangıcı

### Ön koşullar

* Delivery tamamlanmış olmalıdır.
* Bütün slotlar boş olmalıdır.
* FailSafe aktif olmamalıdır.
* Lineer aktüatör hazır olmalıdır.

### Adımlar

1. DELIVERY görevini tamamla.
2. Ana durumu sorgula.
3. Zipline alt durumunu gözlemle.

### Başarı kriterleri

* Ana robot durumu `ZIPLINE` olmalıdır.
* Zipline görevi geçerli başlangıç alt durumuna geçmelidir.
* ARM modu kullanılmamalıdır.
* Sürüş yalnızca `POSITIONING` alt durumunda açık olmalıdır.

---

## TP-ZIP-002 — Uzatma onayı

### Adımlar

Aktüatörün fiziksel uzatması tamamlandıktan sonra gönder:

```text
ZIP,EXTENDED
```

### Beklenen cevap

```text
OK,ZIP,POSITIONING
```

### Başarı kriterleri

* Zipline alt durumu `POSITIONING` olmalıdır.
* Pilot sürüşle hizalama yapabilmelidir.
* Önceki adım tamamlanmadan yanlış sıradaki onay kabul edilmemelidir.

---

## TP-ZIP-003 — Konumlandırma onayı

### Adımlar

Pilot robotu zipline başlangıç noktasına doğru konumlandırdıktan sonra gönder:

```text
ZIP,POSITIONED
```

### Beklenen cevap

```text
OK,ZIP,SLIDING
```

### Başarı kriterleri

* Konumlandırma tamamlandığında sürüş motorları durmalıdır.
* Alt durum `SLIDING` olmalıdır.
* Sliding sırasında joystick sürüşü engellenmelidir.

---

## TP-ZIP-004 — Kayma onayı

### Adımlar

Robot zipline üzerinde kaymayı fiziksel olarak tamamladıktan sonra gönder:

```text
ZIP,SLID
```

### Beklenen cevap

```text
OK,ZIP,RETRACTING
```

### Başarı kriterleri

* Alt durum `RETRACTING` olmalıdır.
* Aktüatör geri çekme işlemi güvenli şekilde başlayabilmelidir.
* Sürüş motorları çalışmamalıdır.

---

## TP-ZIP-005 — Geri çekme onayı

### Adımlar

Aktüatör tamamen geri çekildikten sonra gönder:

```text
ZIP,RETRACTED
```

### Beklenen cevap

```text
OK,ZIP,COMPLETED
```

### Başarı kriterleri

* Zipline alt durumu `COMPLETED` olmalıdır.
* Ana görev sistemi `FINISH` durumuna geçmelidir.
* Bütün hareket çıkışları durmalıdır.

---

## TP-ZIP-006 — Joystick zipline onay butonu

### Bilgi

Zipline onay butonunun indeksi:

```text
3
```

Bit maskesi:

```text
1 << 3 = 8
```

### Adımlar

Her zipline adımında önce buton bırakılmış paket gönder:

```text
JOY,0,0,0,20,-1,0
```

Sonra onay butonlu paket gönder:

```text
JOY,0,0,0,20,-1,8
```

Sonra butonu bırak:

```text
JOY,0,0,0,20,-1,0
```

### Başarı kriterleri

* Her fiziksel basış yalnızca bir alt durumu ilerletmelidir.
* Buton basılı tutulunca birden fazla adım atlanmamalıdır.
* Yanlış ana durumda zipline onayı kabul edilmemelidir.
* POSITIONING başlamadan sürüş açılmamalıdır.
* POSITIONING tamamlanınca sürüş kapanmalıdır.

---

## TP-ZIP-007 — Zipline sırasında FailSafe

### Adımlar

Zipline’ın her alt durumunda ayrı ayrı haberleşme kaybı oluştur:

* EXTENDING
* POSITIONING
* SLIDING
* RETRACTING

### Başarı kriterleri

* Motorlar durmalıdır.
* Aktüatör durmalıdır.
* Sistem FAIL_SAFE durumuna geçmelidir.
* Recovery sırasında güvenli alt durum uygulanmalıdır.
* Sistem otomatik olarak fiziksel adımı tamamlanmış varsaymamalıdır.

---

# 25. Finish testi

## TP-FIN-001 — FINISH durumuna geçiş

### Adımlar

1. Zipline adımlarını doğru sırada tamamla.
2. Ana robot durumunu sorgula.

### Başarı kriterleri

* Ana durum `FINISH` olmalıdır.
* Sürüş motorları durmuş olmalıdır.
* Robot kolu pasif olmalıdır.
* Lineer aktüatör durmuş olmalıdır.
* Görev tamamlandı mesajı verilmelidir.
* OLED mevcutsa tamamlanma ekranı göstermelidir.

---

## TP-FIN-002 — FINISH sonrasında hareket engeli

### Adımlar

1. Robotu FINISH durumuna getir.
2. Joystick hareket komutları gönder.
3. ARM moduna geçmeyi dene.
4. Görev durumunu değiştirmeyi dene.

### Başarı kriterleri

* Robot kendiliğinden yeni göreve başlamamalıdır.
* Motorlar ve robot kolu hareket etmemelidir.
* Yeni görev için açık bir reset veya yeniden başlatma prosedürü gerekmelidir.

---

# 26. Watchdog testleri

## TP-WDG-001 — Yazılımsal watchdog izleme

### Amaç

Ana döngünün beklenen sürede çalıştığını doğrulamak.

### Adımlar

1. Normal çalışma sırasında watchdog hata üretip üretmediğini izle.
2. Kontrollü test sürümünde `feed()` çağrısını geçici olarak geciktir.
3. Timeout davranışını gözlemle.
4. Test sonrası geçici değişikliği geri al.

### Başarı kriterleri

* Normal çalışmada yanlış watchdog hatası oluşmamalıdır.
* Timeout süresi aşıldığında hata algılanmalıdır.
* Sistem güvenli duruma geçmelidir.

### Önemli not

Mevcut `millis()` tabanlı watchdog, ana döngü tamamen kilitlenirse kod çalıştırarak sistemi kurtaramaz. Gerçek ESP32 hardware/task watchdog testi ayrıca planlanmalıdır.

---

# 27. Uzun süreli çalışma testleri

## TP-ENDURANCE-001 — Otuz dakikalık masaüstü testi

### Adımlar

1. Robotu güvenli test standına al.
2. Sistemi otuz dakika açık tut.
3. Düzenli joystick paketleri gönder.
4. DRIVE ve ARM modları arasında kontrollü geçişler yap.
5. RFID ve OLED işlemlerini tekrarla.
6. Seri port çıktılarını kaydet.

### Başarı kriterleri

* Beklenmeyen reset olmamalıdır.
* Seri haberleşme kesilmemelidir.
* OLED donmamalıdır.
* Servo hedefleri bozulmamalıdır.
* Motor kontrolü kilitlenmemelidir.
* Bellek sorununu düşündüren giderek artan kararsızlık olmamalıdır.

---

## TP-ENDURANCE-002 — Tekrarlı görev döngüsü

### Adımlar

Aşağıdaki mantıksal işlemleri en az on kez tekrarla:

1. Slotları doldur.
2. Pickup tamamla.
3. Slotları teslim ederek boşalt.
4. Delivery tamamla.
5. Test ortamında görev verilerini sıfırla.
6. Tekrar başlat.

### Başarı kriterleri

* Slot verileri döngüler arasında karışmamalıdır.
* Eski UID kayıtları yanlışlıkla kalmamalıdır.
* Durum geçişleri tutarlı olmalıdır.
* Sistem reset atmamalıdır.

---

# 28. Tam sistem entegrasyon testi

## TP-SYSTEM-001 — Masaüstü tam sistem testi

### Amaç

Tüm modüllerin birlikte çalışmasını doğrulamak.

### Adımlar

1. Sistemi aç.
2. Başlangıç kontrollerini tamamla.
3. UART ve joystick bağlantısını doğrula.
4. MANUAL durumuna geç.
5. DRIVE modunda motorları test et.
6. ARM moduna geç.
7. Robot kolunu ve kıskacı test et.
8. PICKUP durumuna geç.
9. Beş farklı RFID yükünü slotlara kaydet.
10. DELIVERY durumuna geç.
11. Yükleri doğru slotlardan teslim et.
12. Slotların boşaldığını doğrula.
13. ZIPLINE durumuna geç.
14. Zipline onaylarını sırayla uygula.
15. FINISH durumuna ulaş.
16. Son durumda bütün hareketlerin durduğunu doğrula.

### Başarı kriterleri

* Görev akışı sırası bozulmamalıdır.
* DRIVE ve ARM aynı anda aktif olmamalıdır.
* Beş slotun verileri doğru korunmalıdır.
* RFID ve şehir eşleşmeleri doğru olmalıdır.
* Zipline adımları atlanmamalıdır.
* FailSafe gerektiren bir durumda robot durmalıdır.
* FINISH sonunda bütün hareketler kapatılmalıdır.

---

# 29. Saha testleri

## TP-FIELD-001 — Düşük hızlı sürüş parkuru

### Parkur

* Düz ileri
* Düz geri
* Sağa dönüş
* Sola dönüş
* Dar alanda dönüş
* Hedef noktada durma

### Başarı kriterleri

* Robot joystick komutlarına öngörülebilir tepki vermelidir.
* Düz sürüşte kabul edilemez sapma olmamalıdır.
* Motor sürücüleri aşırı ısınmamalıdır.
* Güç hattı nedeniyle kart reset atmamalıdır.
* Pilot robotu hassas biçimde durdurabilmelidir.

---

## TP-FIELD-002 — Yük alma testi

### Adımlar

1. Robotu yükün yanına getir.
2. ARM moduna geç.
3. Kıskacı aç.
4. Kolu yükün konumuna getir.
5. Yükü kavra.
6. Yükü kaldır.
7. Slota yerleştir.
8. RFID ve slot kaydını doğrula.

### Başarı kriterleri

* Robot kolu yükü güvenli şekilde kavramalıdır.
* Yük düşmemelidir.
* Servo sürekli zorlanmamalıdır.
* Yük doğru slota yerleştirilmelidir.
* RFID kaydı fiziksel yükle eşleşmelidir.

---

## TP-FIELD-003 — Yük teslim testi

### Adımlar

1. Teslim noktasına sür.
2. RFID ile yükü doğrula.
3. Doğru slotu belirle.
4. ARM moduna geç.
5. Yükü slottan çıkar.
6. Teslim noktasına bırak.
7. Slot kaydını pilot onayı sonrasında temizle.

### Başarı kriterleri

* Yanlış slot seçilmemelidir.
* Slot, yük fiziksel olarak bırakılmadan silinmemelidir.
* Bırakılan yük yeniden kayıtlı görünmemelidir.

---

## TP-FIELD-004 — Gerçek zipline testi

### Ön koşullar

Aşağıdaki testlerin tamamı PASS olmalıdır:

* Lineer aktüatör testleri
* FailSafe testi
* Düşük hızlı sürüş testi
* Mekanik zipline kontrolü
* Aktüatör hareket mesafesi kontrolü

### Adımlar

1. Robotu zipline başlangıç alanına getir.
2. Delivery görevini tamamla.
3. Zipline moduna geç.
4. Aktüatörü uzat.
5. Fiziksel uzatma tamamlandıktan sonra pilot onayı ver.
6. Robotu joystick ile hizala.
7. Konumlandırma onayı ver.
8. Robotun güvenli şekilde kaymasını sağla.
9. Kayma tamamlanınca onay ver.
10. Aktüatörü geri çek.
11. Geri çekme tamamlanınca onay ver.
12. FINISH durumunu doğrula.

### Başarı kriterleri

* Zipline adımları doğru sırada ilerlemelidir.
* Robot konumlandırma dışındaki adımlarda sürülmemelidir.
* Aktüatör mekanik sınıra zorlanmamalıdır.
* Robot zipline üzerinde dengeli kalmalıdır.
* FailSafe durumunda hareketler durmalıdır.
* Görev FINISH ile tamamlanmalıdır.

---

# 30. Regresyon testleri

Aşağıdaki dosyalardan biri değiştirildiğinde ilgili testler tekrar uygulanmalıdır.

| Değiştirilen bölüm | Tekrarlanacak testler                                  |
| ------------------ | ------------------------------------------------------ |
| `Pins.h`           | Tüm donanım başlangıç ve hareket testleri              |
| `MotorDriver`      | TP-DRIVE-001–008                                       |
| `MotionController` | TP-DRIVE-002–008, TP-SAFE-001                          |
| `Joystick`         | TP-JOY-001–005, TP-MODE-003, TP-ZIP-006                |
| `ServoDriver`      | TP-BOOT-003, TP-ARM-001–008                            |
| `ServoManager`     | TP-ARM-001–008                                         |
| `RobotArm`         | TP-MODE-002–003, TP-ARM-002–008                        |
| `RFID`             | TP-RFID-001–004, pickup ve delivery testleri           |
| `SlotManager`      | TP-SLOT-001–008, pickup ve delivery testleri           |
| `StateMachine`     | Bütün görev geçiş testleri                             |
| `MissionManager`   | Pickup, delivery, zipline, FailSafe ve Finish testleri |
| `Zipline`          | TP-ZIP-001–007                                         |
| `Finish`           | TP-FIN-001–002                                         |
| `PowerManager`     | TP-POWER-001–003                                       |
| `FailSafe`         | TP-SAFE-001–004                                        |
| `main.cpp`         | Derleme, başlangıç, UART ve tam sistem testi           |

Bir hata düzeltildiğinde yalnızca hatalı test değil, değişiklikten etkilenebilecek bütün regresyon testleri çalıştırılmalıdır.

---

# 31. Yarışma öncesi kontrol listesi

## Yazılım

* [ ] Kod temiz şekilde derleniyor.
* [ ] Test edilen commit GitHub’a gönderildi.
* [ ] Yerel commitlenmemiş değişiklik yok.
* [ ] Seri port başlangıç mesajları doğru.
* [ ] DRIVE modu çalışıyor.
* [ ] ARM modu çalışıyor.
* [ ] Mod geçişi güvenli.
* [ ] Joystick timeout FailSafe çalışıyor.
* [ ] Recovery çalışıyor.
* [ ] RFID okuma çalışıyor.
* [ ] Beş slot doğru çalışıyor.
* [ ] Pickup akışı çalışıyor.
* [ ] Delivery akışı çalışıyor.
* [ ] Zipline akışı çalışıyor.
* [ ] Finish durumu çalışıyor.

## Elektronik

* [ ] Bütün pinler kart şemasıyla doğrulandı.
* [ ] Pin çakışması yok.
* [ ] Altı BTS7960 sürüş kanalı çalışıyor.
* [ ] Lineer aktüatör pinleri atandı.
* [ ] Lineer aktüatör yönleri doğru.
* [ ] OLED çalışıyor.
* [ ] RFID çalışıyor.
* [ ] Servo güç bağlantısı güvenli.
* [ ] Batarya ADC bağlantısı yapıldı.
* [ ] Batarya ölçümü kalibre edildi.
* [ ] Kablolarda gevşek bağlantı yok.
* [ ] Acil güç kesme yöntemi hazır.

## Mekanik

* [ ] Altı tekerlek serbest dönüyor.
* [ ] Robot düz sürüş yapıyor.
* [ ] Robot kolu mekanik sınıra çarpmıyor.
* [ ] Kıskaç yükü güvenli tutuyor.
* [ ] Beş slot yükleri güvenli taşıyor.
* [ ] Lineer aktüatör sıkışmıyor.
* [ ] Zipline bağlantısı güvenli.
* [ ] Robotun ağırlık merkezi uygundur.

## Operasyon

* [ ] Pilot DRIVE/ARM mod geçişini biliyor.
* [ ] Pilot zipline onay sırasını biliyor.
* [ ] FailSafe recovery prosedürü biliniyor.
* [ ] Yedek USB kablosu hazır.
* [ ] Yedek sigorta ve bağlantı malzemeleri hazır.
* [ ] Bataryalar dolu ve güvenli durumda.
* [ ] Test sonuçları kayıt altına alındı.

---

# 32. Genel kabul kriterleri

METEBOT yazılımı robot üzerinde kullanıma hazır kabul edilmeden önce:

1. Bütün P0 güvenlik testleri PASS olmalıdır.
2. Sürüş motorlarının tamamı doğru yönde çalışmalıdır.
3. DRIVE ve ARM aynı anda hareket üretmemelidir.
4. Haberleşme kaybında robot güvenli şekilde durmalıdır.
5. Robot kolu mekanik sınırları aşmamalıdır.
6. RFID ve slot kayıtları doğru eşleşmelidir.
7. Pickup ve delivery akışı veri kaybı olmadan tamamlanmalıdır.
8. Lineer aktüatör güvenli biçimde uzayıp geri çekilmelidir.
9. Zipline adımları yanlış sırada ilerlememelidir.
10. FINISH durumunda bütün hareketler durmalıdır.
11. Otuz dakikalık çalışma testinde beklenmeyen reset olmamalıdır.
12. Tam görev akışı en az üç kez arka arkaya başarıyla tamamlanmalıdır.
13. Test edilen yazılım sürümü Git commit numarasıyla kayıtlı olmalıdır.

Bu kriterlerden biri sağlanmıyorsa robot yarışmaya hazır kabul edilmemelidir.


## TP-ARM-WRIST-001 — Bilek servo hat switch kontrol testi

Ön koşullar:

- Kontrol modu `ARM` olmalıdır.
- Bilek servosu D12 sinyal pinine bağlanmalıdır.
- Buton 4, bit maskesinde indeks 4 olduğu için değeri `16`dır.

Adımlar:

1. Buton bırakılmış paket gönder: `JOY,0,0,0,50,-1,0`
2. Buton 4'e bas: `JOY,0,0,0,50,-1,16`
3. Butonu bırak: `JOY,0,0,0,50,-1,0`
4. Buton 4'e yeniden bas: `JOY,0,0,0,50,-1,16`

Beklenen sonuç:

- Hat switch yukarı tutulduğunda bilek hedef açısı artar.
- Hat switch aşağı tutulduğunda bilek hedef açısı azalır.
- Hat switch bırakıldığında hedef açı değişmez.
- Servo, yapılandırılmış minimum ve maksimum açıları aşmaz.
- Buton basılı tutulduğunda her pakette tekrar tekrar konum değiştirmez.
- Servo hedefleri `JointConfig` ve `Pins::SERVO_PINS` limitleri dışına çıkmaz.


### Kiskac basili tutma kontrol testi

1. Robotu ARM moduna alin.
2. Kiskac acma butonuna kisa sure basin; kiskac yalnizca kucuk bir aci kadar hareket etmelidir.
3. Ayni butonu basili tutun; kiskac basili kaldigi surece kademeli olarak acilmalidir.
4. Butonu birakin; kiskac o anki hedef acida durmalidir.
5. Kapatma butonuyla ayni testi ters yonde uygulayin.
6. Iki butona ayni anda basin; kiskac hareket etmemelidir.
7. Minimum ve maksimum fiziksel limitlerde buton basili kalsa bile hedef aci limiti asmamalidir.
8. Wi-Fi paketi kesildiginde yeni kiskac hareketi uretilmemelidir.
