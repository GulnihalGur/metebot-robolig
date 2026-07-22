#pragma once

#include <Arduino.h>
#include "Types.h"

// Robot uzerindeki yuk slotlarini yonetir.
class SlotManager
{
public:
    static constexpr uint8_t MAX_SLOTS = 8;
    static constexpr uint8_t DEFAULT_SLOT_COUNT = 5;
    static constexpr int8_t INVALID_SLOT = -1;

    SlotManager();

    // Slotlari baslatir ve temizler.
    bool begin(uint8_t slotCount = DEFAULT_SLOT_COUNT);

    // Yuku tercih edilen veya ilk bos slota yerlestirir.
    bool placeItem(
        const String& uid,
        City city,
        int8_t preferredSlot = INVALID_SLOT
    );

    // Belirtilen slotu bosaltir.
    bool removeItem(uint8_t slotIndex);

    // UID ile bulunan slotu bosaltir.
    bool removeByUID(const String& uid);

    // Tum slotlari temizler.
    void clearAll();

    // Slot arama islemleri.
    int8_t findEmptySlot() const;
    int8_t findSlotByUID(const String& uid) const;
    int8_t findSlotByCity(City city) const;

    // Slot durum sorgulari.
    bool isOccupied(uint8_t slotIndex) const;
    bool isUIDStored(const String& uid) const;

    bool full() const;
    bool isEmpty() const;
    bool ready() const;

    uint8_t slotCount() const;
    uint8_t occupiedCount() const;
    int8_t lastChangedSlot() const;

    // Gecersiz indekste nullptr dondurur.
    const SlotInfo* slot(uint8_t slotIndex) const;

private:
    SlotInfo _slots[MAX_SLOTS];

    uint8_t _slotCount;
    uint8_t _occupiedCount;
    int8_t _lastChangedSlot;
    bool _ready;

    bool validIndex(uint8_t slotIndex) const;

    // UID bicimini standartlastirir.
    static String normalizeUID(const String& uid);
};