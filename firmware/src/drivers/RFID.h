#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>
#include <PN532_HSU.h>
#include <PN532.h>
#include <Pins.h>

class RFID
{
public:
    static constexpr uint8_t MAX_UID_LENGTH = 10;

    explicit RFID(HardwareSerial& serialPort = Serial2);

    bool begin(
        uint32_t baud = Pins::RFID_BAUD,
        int rxPin = Pins::RFID_RX,
        int txPin = Pins::RFID_TX
    );

    bool readUID(
        uint8_t* uidBuffer,
        uint8_t* uidLength,
        uint16_t timeoutMs = 100
    );

    String readUIDString(uint16_t timeoutMs = 100);

    bool ready() const;
    uint32_t firmwareVersion() const;

private:
    HardwareSerial& _serial;
    PN532_HSU _pn532Interface;
    PN532 _pn532;

    bool _ready;
    uint32_t _firmwareVersion;

    static String uidToString(
        const uint8_t* uid,
        uint8_t uidLength
    );
};