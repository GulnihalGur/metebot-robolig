# PC joystick - Wi-Fi/UDP

## Baglanti

1. Deneyap Kart acildiginda `METEBOT` adinda Wi-Fi agi olusturur.
2. PC bu aga baglanir.
3. Varsayilan sifre: `metebot123`
4. Kart IP adresi: `192.168.4.1`
5. UDP portu: `4210`

Joystick Deneyap pinlerine baglanmaz:

```text
Joystick -> USB -> PC -> Wi-Fi/UDP -> Deneyap Kart
```

## Kurulum

```powershell
cd tools
py -m pip install -r requirements.txt
```

## Calistirma

```powershell
py pc_joystick_wifi.py --show
```

Eksen numaralari joystick modelinde farkliysa:

```powershell
py pc_joystick_wifi.py --x-axis 0 --y-axis 1 --twist-axis 2 --throttle-axis 3 --show
```

## Butonlar

- Fiziksel buton 1: gripper ac
- Fiziksel buton 2: gripper kapat
- Fiziksel buton 3: DRIVE/ARM modu
- Fiziksel buton 4: zipline onayi

## Bilek kontrolu

- Joystick uzerindeki hat switch yukari: bilegi bir yone hareket ettirir.
- Hat switch asagi: bilegi ters yone hareket ettirir.
- Hat switch birakildiginda bilek mevcut hedef acida durur.
- Hat switch sag/sol hareketleri bilegi etkilemez.

## Guvenlik

Kart `Pins::JOYSTICK_TIMEOUT_MS` boyunca paket alamazsa iletisim FailSafe'i calisir. Python kapanirken uc adet notr paket gonderir; asil guvenlik kart tarafindaki timeout'tur.


## Kiskac buton davranisi

- Fiziksel buton 1 basili tutuldukca kiskac yavasca acilir.
- Fiziksel buton 2 basili tutuldukca kiskac yavasca kapanir.
- Buton birakilinca kiskac mevcut hedef acida durur.
- Iki buton ayni anda basilirsa kiskac hareket etmez.
- Python paket formati degismemistir; buton bit maskesi basili kalma bilgisini zaten her pakette gonderir.
