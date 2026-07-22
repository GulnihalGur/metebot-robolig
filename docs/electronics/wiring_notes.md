# METEBOT Kablolama Notları

## Sürüş BTS7960 bağlantı düzeni

Her teker motoru kendi BTS7960 sürücüsünü kullanır. Sadece kontrol girişleri gruplanır.

```text
D0  -> Sol ön BTS RPWM
D1  -> Sol ön BTS LPWM

A2  -> Sol orta BTS RPWM + Sol arka BTS RPWM
A3  -> Sol orta BTS LPWM + Sol arka BTS LPWM

D12 -> Sağ ön BTS RPWM
D13 -> Sağ ön BTS LPWM

A5  -> Sağ orta BTS RPWM + Sağ arka BTS RPWM
A6  -> Sağ orta BTS LPWM + Sağ arka BTS LPWM
```

### Kesinlikle ortaklanmayacak bağlantılar

- Farklı BTS'lerin `M+` ve `M-` motor çıkışları birbirine bağlanmaz.
- İki motor tek BTS çıkışına bağlanmaz.
- BTS güç çıkışları kontrol pini gibi paralellenmez.

### Ortaklanacak bağlantılar

- Aynı bogie grubundaki iki BTS'nin `RPWM` girişleri ortak kontrol pinine bağlanır.
- Aynı bogie grubundaki iki BTS'nin `LPWM` girişleri ortak kontrol pinine bağlanır.
- Tüm kontrol devrelerinin GND hattı ortak olmalıdır.

## BTS enable girişleri

BTS7960 modülünde `R_EN` ve `L_EN` bulunuyorsa üretici modülüne göre lojik HIGH seviyesinde tutulmalıdır. Sabit enable kullanılacaksa güvenilir 3.3 V bağlantısı ve ortak GND sağlanmalıdır. Daha sonra donanımsal fail-safe istenirse enable pinleri ayrı bir güvenlik devresine taşınabilir.

## Yön doğrulaması

Aynı kontrol hattını kullanan orta ve arka motorlardan biri ters dönüyorsa yazılımda ayrı tersleme yapılamaz. O BTS'nin motor tarafındaki `M+` ve `M-` kabloları yer değiştirilmelidir. Güç kesilmeden kablo değişikliği yapılmamalıdır.

## Güç dağıtımı

- BTS `B+ / B-`: motor batarya hattına uygun sigorta ve kablo kesiti üzerinden bağlanır.
- BTS lojik `VCC`: kullanılan modülün gerektirdiği regüle lojik beslemeye bağlanır.
- Servo beslemesi kartın 3.3 V pininden verilmez; uygun UBEC kullanılmalıdır.
- Motor, servo ve aktüatör akımları Deneyap kart üzerinden geçirilmez.
- USB bağlıyken güç kaynaklarının geri besleme oluşturmadığı doğrulanmalıdır.

## İlk çalıştırma sırası

1. Batarya bağlı değilken süreklilik ve kısa devre kontrolü yapın.
2. BTS kontrol pinlerini tek tek doğrulayın.
3. Tekerlekleri yerden kaldırın.
4. Her mantıksal grubu düşük PWM ile ayrı test edin.
5. Sol ve sağ motor yönlerini doğrulayın.
6. Bogie grubunda orta ve arkanın birlikte fakat doğru yönde döndüğünü doğrulayın.
7. Son olarak yerde düşük hızlı sürüş testi yapın.
