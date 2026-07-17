#pragma once

#include <Arduino.h>

// Bir slotta tutulan yuk bilgisidir.
struct SlotInfo
{
    bool occupied;
    String uid;
    String city;
};

// Robot uzerindeki yuk slotlarini yonetir.
class SlotManager
{
public:
    static constexpr uint8_t MAX_SLOTS = 8;

    // Robot tasariminda 5 fiziksel slot bulunuyor.
    static constexpr uint8_t DEFAULT_SLOT_COUNT = 5;

    static constexpr int8_t INVALID_SLOT = -1;

    SlotManager();

    // Kullanilacak slot sayisini ayarlar ve tum slotlari temizler.
    bool begin(uint8_t slotCount = DEFAULT_SLOT_COUNT);

    /*
     * PICKUP:
     * Okunan UID ve sehir bilgisini tercih edilen veya ilk bos slota yerlestirir.
     */
    bool placeItem(
        const String& uid,
        const String& city,
        int8_t preferredSlot = INVALID_SLOT
    );

    /*
     * DELIVERY:
     * Belirtilen slotu bosaltir.
     */
    bool removeItem(uint8_t slotIndex);

    /*
     * DELIVERY:
     * RFID ile okunan UID'nin bulundugu slotu bulur ve bosaltir.
     */
    bool removeByUID(const String& uid);

    // Tum slotlari bosaltir.
    void clearAll();

    // Arama fonksiyonlari.
    int8_t findEmptySlot() const;
    int8_t findSlotByUID(const String& uid) const;
    int8_t findSlotByCity(const String& city) const;

    // Durum sorgulari.
    bool isOccupied(uint8_t slotIndex) const;
    bool isUIDStored(const String& uid) const;

    bool full() const;
    bool isEmpty() const;
    bool ready() const;

    uint8_t slotCount() const;
    uint8_t occupiedCount() const;
    int8_t lastChangedSlot() const;

    // Gecersiz indeks icin nullptr dondurur.
    const SlotInfo* slot(uint8_t slotIndex) const;

private:
    SlotInfo _slots[MAX_SLOTS];

    uint8_t _slotCount;
    uint8_t _occupiedCount;

    int8_t _lastChangedSlot;

    bool _ready;

    bool validIndex(uint8_t slotIndex) const;

    static String normalizeUID(const String& uid);
};