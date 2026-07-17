#include "RFID.h"

RFID::RFID(HardwareSerial& serialPort)
    : _serial(serialPort),
      _pn532Interface(serialPort),
      _pn532(_pn532Interface),
      _ready(false),
      _firmwareVersion(0)
{
}

bool RFID::begin(uint32_t baud, int rxPin, int txPin)
{
    _serial.begin(baud, SERIAL_8N1, rxPin, txPin);
    delay(50);

    _pn532.begin();

    _firmwareVersion = _pn532.getFirmwareVersion();

    if (_firmwareVersion == 0)
    {
        _ready = false;
        return false;
    }

    _pn532.SAMConfig();

    _ready = true;
    return true;
}

bool RFID::readUID(
    uint8_t* uidBuffer,
    uint8_t* uidLength,
    uint16_t timeoutMs
)
{
    if (!_ready ||
        uidBuffer == nullptr ||
        uidLength == nullptr)
    {
        return false;
    }

    bool success = _pn532.readPassiveTargetID(
        PN532_MIFARE_ISO14443A,
        uidBuffer,
        uidLength,
        timeoutMs
    );

    if (!success ||
        *uidLength == 0 ||
        *uidLength > MAX_UID_LENGTH)
    {
        return false;
    }

    return true;
}

String RFID::readUIDString(uint16_t timeoutMs)
{
    uint8_t uid[MAX_UID_LENGTH];
    uint8_t uidLength = 0;

    if (!readUID(uid, &uidLength, timeoutMs))
    {
        return "";
    }

    return uidToString(uid, uidLength);
}

bool RFID::ready() const
{
    return _ready;
}

uint32_t RFID::firmwareVersion() const
{
    return _firmwareVersion;
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