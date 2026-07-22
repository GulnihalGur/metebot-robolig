# METEBOT Kodlama Kuralları

## Genel

- Donanım pinleri yalnızca `firmware/include/Pins.h` içinde tanımlanır.
- Sabit sayılar mümkün olduğunca isimlendirilmiş `constexpr` değerler olmalıdır.
- Bloklayıcı uzun `delay()` çağrıları ana kontrol akışında kullanılmamalıdır.
- Her sürücü `begin()` sonucu ile hazır olup olmadığını bildirmelidir.
- Pointer kullanılmadan önce `nullptr`, kanal kullanılmadan önce indeks sınırı kontrol edilmelidir.

## Motor kontrolü

- BTS çıkışlarına doğrudan başka modüllerden `analogWrite()` yapılmaz; yalnızca `MotorDriver` kullanılır.
- Normal sürüş için `setDriveSpeeds(left, right)` kullanılmalıdır.
- Ön/bogie bağımsız kontrolü yalnızca açıkça tasarlanmış bir tırmanma veya kurtarma algoritmasında `setDriveGroupSpeeds()` ile yapılmalıdır.
- Durdurma sırasında dört mantıksal kanalın tamamı sıfırlanmalıdır.
- Ortak kontrol hattındaki tek motor ters dönüyorsa yazılımsal kanal tersleme eklenmez; fiziksel motor kablosu düzeltilir.

## Yorum ve adlandırma

- Kod yorumları kısa Türkçe ve mümkün olduğunca ASCII karakterlerle yazılır.
- Sınıf adları `PascalCase`, fonksiyon ve değişkenler `camelCase`, sabitler `UPPER_SNAKE_CASE` kullanır.
- Fonksiyon adı davranışı açıkça belirtmelidir; `setLeftBogie()` gibi fiziksel grubu gizlememelidir.

## Değişiklik kontrolü

Pin, görev akışı veya güvenlik davranışı değiştiğinde aynı PR içinde ilgili dokümanlar ve test planı da güncellenmelidir. Derleme yapılmadan PR hazır kabul edilmemelidir.
