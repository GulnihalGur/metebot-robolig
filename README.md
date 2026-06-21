
# METEBOT Robolig 2026

METEBOT, ALTAY takımı tarafından TEKNOFEST 2026 Robolig yarışması için geliştirilen robotun gömülü kontrol yazılımıdır.

## Ana Kontrol Kartı

* Deneyap Kart 1A v2 (ESP32 tabanlı)

## Temel Yazılım Modülleri

* Motor kontrol sistemi
* Lineer aktüatör kontrol sistemi
* RFID okuma sistemi
* OLED ekran yönetimi
* Seri haberleşme tabanlı joystick kontrolü
* Fail-safe (güvenli duruş) sistemi
* Non-blocking state machine (engellemeyen durum makinesi) mimarisi

## Yazılım Mimarisi

Yazılım, görev tabanlı ve modüler bir yapı üzerine geliştirilmiştir. Tüm alt sistemler eş zamanlı çalışabilecek şekilde non-blocking state machine yaklaşımıyla tasarlanmaktadır.

## Geliştirme Kuralları

* `main` dalına doğrudan kod gönderilmez.
* Her yeni özellik için ayrı bir branch oluşturulur.
* Donanım modülleri entegrasyon öncesinde ayrı ayrı test edilir.
* Güvenlik fonksiyonları, otonom görev mantığından önce geliştirilir.
* Tüm kod değişiklikleri Pull Request üzerinden incelenir.

## Geliştirme Ekibi

* Yazılım Geliştirici: Gül Nihal Gür
* Yazılım Geliştirici: Ayşe Beyza Nur Binar

## Yarışma

* Yarışma: TEKNOFEST 2026 Robolig
* Takım Adı: ALTAY
* Proje Adı: METEBOT
