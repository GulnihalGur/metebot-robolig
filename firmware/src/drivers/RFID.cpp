#include "RFID.h"

#include <string.h>

RFID::RFID(HardwareSerial& serialPort)
    : _serial(serialPort),
      _pn532Interface(serialPort),
      _pn532(_pn532Interface),
      _ready(false),
      _firmwareVersion(0),
      _lastReadStatus(RFIDReadStatus::HARDWARE_ERROR)
{
}

bool RFID::begin(uint32_t baud, int rxPin, int txPin)
{
    _ready = false;
    _firmwareVersion = 0;
    _lastReadStatus = RFIDReadStatus::HARDWARE_ERROR;

    _serial.begin(baud, SERIAL_8N1, rxPin, txPin);
    delay(50);

    _pn532.begin();

    _firmwareVersion = _pn532.getFirmwareVersion();

    if (_firmwareVersion == 0)
    {
        return false;
    }

    if (!_pn532.SAMConfig())
    {
        return false;
    }

    /*
     * Varsayilan 0xFF PN532'nin kart gelene kadar beklemesine neden olabilir.
     * 0x00 tek ve kisa bir pasif aktivasyon denemesi yapar. Boylece normal
     * "kart yok" cevabi host timeout'una dusmeden donebilir.
     */
    if (!_pn532.setPassiveActivationRetries(PASSIVE_ACTIVATION_RETRIES))
    {
        return false;
    }

    _ready = true;
    _lastReadStatus = RFIDReadStatus::NO_TAG;
    return true;
}

RFIDReadStatus RFID::readUID(
    uint8_t* uidBuffer,
    uint8_t* uidLength,
    uint16_t timeoutMs
)
{
    if (uidLength != nullptr)
    {
        *uidLength = 0;
    }

    if (uidBuffer != nullptr)
    {
        memset(uidBuffer, 0, MAX_UID_LENGTH);
    }

    if (!_ready)
    {
        return setLastStatus(RFIDReadStatus::HARDWARE_ERROR);
    }

    if (uidBuffer == nullptr || uidLength == nullptr)
    {
        return setLastStatus(RFIDReadStatus::INVALID_UID);
    }

    const uint16_t safeTimeoutMs = constrain(
        timeoutMs,
        static_cast<uint16_t>(1),
        MAX_READ_TIMEOUT_MS
    );

    const uint8_t command[] = {
        PN532_COMMAND_INLISTPASSIVETARGET,
        1,
        PN532_MIFARE_ISO14443A
    };

    /*
     * readPassiveTargetID() yalnizca bool dondurdugu icin hata nedenini
     * kaybediyordu. Alt seviye arayuz kullanilarak ACK, timeout ve normal
     * "0 kart" cevabi birbirinden ayrilir.
     */
    const int8_t writeResult = _pn532Interface.writeCommand(
        command,
        sizeof(command)
    );

    if (writeResult != 0)
    {
        return setLastStatus(RFIDReadStatus::HARDWARE_ERROR);
    }

    uint8_t response[RESPONSE_BUFFER_LENGTH] = {0};

    const int16_t responseLength = _pn532Interface.readResponse(
        response,
        sizeof(response),
        safeTimeoutMs
    );

    if (responseLength == PN532_TIMEOUT)
    {
        return setLastStatus(RFIDReadStatus::TIMEOUT);
    }

    if (responseLength < 0)
    {
        return setLastStatus(RFIDReadStatus::HARDWARE_ERROR);
    }

    if (responseLength < 1)
    {
        return setLastStatus(RFIDReadStatus::HARDWARE_ERROR);
    }

    const uint8_t tagCount = response[0];

    if (tagCount == 0)
    {
        return setLastStatus(RFIDReadStatus::NO_TAG);
    }

    if (tagCount != 1)
    {
        return setLastStatus(RFIDReadStatus::HARDWARE_ERROR);
    }

    /*
     * ISO14443A cevabinda UID uzunlugu response[5], UID ise response[6]
     * konumundan baslar. Bu alanlara erismeden once cerceve uzunlugu kontrol edilir.
     */
    if (responseLength < 6)
    {
        return setLastStatus(RFIDReadStatus::INVALID_UID);
    }

    const uint8_t receivedUidLength = response[5];

    if (receivedUidLength == 0 ||
        receivedUidLength > MAX_UID_LENGTH ||
        responseLength < static_cast<int16_t>(6 + receivedUidLength))
    {
        return setLastStatus(RFIDReadStatus::INVALID_UID);
    }

    memcpy(uidBuffer, response + 6, receivedUidLength);
    *uidLength = receivedUidLength;

    return setLastStatus(RFIDReadStatus::READ_SUCCESS);
}

RFIDReadStatus RFID::readUIDString(
    String& outUid,
    uint16_t timeoutMs
)
{
    outUid = "";

    uint8_t uid[MAX_UID_LENGTH] = {0};
    uint8_t uidLength = 0;

    const RFIDReadStatus status = readUID(
        uid,
        &uidLength,
        timeoutMs
    );

    if (status == RFIDReadStatus::READ_SUCCESS)
    {
        outUid = uidToString(uid, uidLength);
    }

    return status;
}

bool RFID::ready() const
{
    return _ready;
}

uint32_t RFID::firmwareVersion() const
{
    return _firmwareVersion;
}

RFIDReadStatus RFID::lastReadStatus() const
{
    return _lastReadStatus;
}

const char* RFID::statusName(RFIDReadStatus status)
{
    switch (status)
    {
        case RFIDReadStatus::NO_TAG:
            return "NO_TAG";

        case RFIDReadStatus::READ_SUCCESS:
            return "READ_SUCCESS";

        case RFIDReadStatus::HARDWARE_ERROR:
            return "HARDWARE_ERROR";

        case RFIDReadStatus::INVALID_UID:
            return "INVALID_UID";

        case RFIDReadStatus::TIMEOUT:
            return "TIMEOUT";
    }

    return "HARDWARE_ERROR";
}

RFIDReadStatus RFID::setLastStatus(RFIDReadStatus status)
{
    _lastReadStatus = status;
    return status;
}

String RFID::uidToString(
    const uint8_t* uid,
    uint8_t uidLength
)
{
    String result;

    for (uint8_t i = 0; i < uidLength; i++)
    {
        if (uid[i] < 0x10)
        {
            result += "0";
        }

        result += String(uid[i], HEX);

        if (i + 1 < uidLength)
        {
            result += ":";
        }
    }

    result.toUpperCase();
    return result;
}
