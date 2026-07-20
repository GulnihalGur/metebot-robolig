#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>
#include <PN532_HSU.h>
#include <PN532.h>
#include <Pins.h>

// RFID okumasinin sonucunu bool yerine acik durumlarla bildirir.
enum class RFIDReadStatus : uint8_t
{
    NO_TAG,
    READ_SUCCESS,
    HARDWARE_ERROR,
    INVALID_UID,
    TIMEOUT
};

class RFID
{
public:
    static constexpr uint8_t MAX_UID_LENGTH = 10;

    // HSU writeCommand ACK beklemesi de ek sure kullanir. Bu nedenle okuma
    // cevabi icin verilen sure kisa ve ust sinirli tutulur.
    static constexpr uint16_t DEFAULT_READ_TIMEOUT_MS = 15;
    static constexpr uint16_t MAX_READ_TIMEOUT_MS = 20;

    explicit RFID(HardwareSerial& serialPort = Serial2);

    bool begin(
        uint32_t baud = Pins::RFID_BAUD,
        int rxPin = Pins::RFID_RX,
        int txPin = Pins::RFID_TX
    );

    /*
     * Tek bir ISO14443A taramasi yapar.
     * timeoutMs yanlislikla buyuk verilse bile MAX_READ_TIMEOUT_MS ile sinirlanir.
     */
    RFIDReadStatus readUID(
        uint8_t* uidBuffer,
        uint8_t* uidLength,
        uint16_t timeoutMs = DEFAULT_READ_TIMEOUT_MS
    );

    /*
     * Basarili okumada UID metnini outUid icine yazar.
     * Basarisiz sonuclarda outUid bos birakilir.
     */
    RFIDReadStatus readUIDString(
        String& outUid,
        uint16_t timeoutMs = DEFAULT_READ_TIMEOUT_MS
    );

    bool ready() const;
    uint32_t firmwareVersion() const;
    RFIDReadStatus lastReadStatus() const;

    static const char* statusName(RFIDReadStatus status);

private:
    // PN532'nin tek taramada uzun sure kart beklememesi icin en dusuk retry.
    static constexpr uint8_t PASSIVE_ACTIVATION_RETRIES = 0x00;
    static constexpr uint8_t RESPONSE_BUFFER_LENGTH = 6 + MAX_UID_LENGTH;

    HardwareSerial& _serial;
    PN532_HSU _pn532Interface;
    PN532 _pn532;

    bool _ready;
    uint32_t _firmwareVersion;
    RFIDReadStatus _lastReadStatus;

    RFIDReadStatus setLastStatus(RFIDReadStatus status);

    static String uidToString(
        const uint8_t* uid,
        uint8_t uidLength
    );
};
