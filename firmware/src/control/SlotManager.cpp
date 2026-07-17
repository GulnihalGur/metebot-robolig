#include "SlotManager.h"

SlotManager::SlotManager()
    : _slotCount(0),
      _occupiedCount(0),
      _lastChangedSlot(INVALID_SLOT),
      _ready(false)
{
    clearAll();
}

bool SlotManager::begin(uint8_t slotCountValue)
{
    if (slotCountValue == 0 || slotCountValue > MAX_SLOTS)
    {
        _ready = false;
        return false;
    }

    _slotCount = slotCountValue;
    _ready = true;

    clearAll();

    return true;
}

bool SlotManager::placeItem(
    const String& uid,
    const String& city,
    int8_t preferredSlot
)
{
    if (!_ready)
    {
        return false;
    }

    String cleanUID = normalizeUID(uid);

    String cleanCity = city;
    cleanCity.trim();

    // Bos UID veya bos sehir bilgisi kabul edilmez.
    if (cleanUID.isEmpty() || cleanCity.isEmpty())
    {
        return false;
    }

    // Ayni koli iki kez kaydedilemez.
    if (isUIDStored(cleanUID))
    {
        return false;
    }

    // Tum slotlar doluysa yeni koli eklenemez.
    if (full())
    {
        return false;
    }

    int8_t targetSlot = INVALID_SLOT;

    if (preferredSlot != INVALID_SLOT)
    {
        if (preferredSlot < 0)
        {
            return false;
        }

        uint8_t index = static_cast<uint8_t>(preferredSlot);

        if (!validIndex(index) || _slots[index].occupied)
        {
            return false;
        }

        targetSlot = preferredSlot;
    }
    else
    {
        targetSlot = findEmptySlot();
    }

    if (targetSlot == INVALID_SLOT)
    {
        return false;
    }

    SlotInfo& selected =
        _slots[static_cast<uint8_t>(targetSlot)];

    selected.occupied = true;
    selected.uid = cleanUID;
    selected.city = cleanCity;

    _occupiedCount++;
    _lastChangedSlot = targetSlot;

    return true;
}

bool SlotManager::removeItem(uint8_t slotIndex)
{
    if (!_ready || !validIndex(slotIndex))
    {
        return false;
    }

    if (!_slots[slotIndex].occupied)
    {
        return false;
    }

    _slots[slotIndex].occupied = false;
    _slots[slotIndex].uid = "";
    _slots[slotIndex].city = "";

    if (_occupiedCount > 0)
    {
        _occupiedCount--;
    }

    _lastChangedSlot =
        static_cast<int8_t>(slotIndex);

    return true;
}

bool SlotManager::removeByUID(const String& uid)
{
    int8_t index = findSlotByUID(uid);

    if (index == INVALID_SLOT)
    {
        return false;
    }

    return removeItem(
        static_cast<uint8_t>(index)
    );
}

void SlotManager::clearAll()
{
    for (uint8_t i = 0; i < MAX_SLOTS; i++)
    {
        _slots[i].occupied = false;
        _slots[i].uid = "";
        _slots[i].city = "";
    }

    _occupiedCount = 0;
    _lastChangedSlot = INVALID_SLOT;
}

int8_t SlotManager::findEmptySlot() const
{
    if (!_ready)
    {
        return INVALID_SLOT;
    }

    for (uint8_t i = 0; i < _slotCount; i++)
    {
        if (!_slots[i].occupied)
        {
            return static_cast<int8_t>(i);
        }
    }

    return INVALID_SLOT;
}

int8_t SlotManager::findSlotByUID(
    const String& uid
) const
{
    if (!_ready)
    {
        return INVALID_SLOT;
    }

    String cleanUID = normalizeUID(uid);

    if (cleanUID.isEmpty())
    {
        return INVALID_SLOT;
    }

    for (uint8_t i = 0; i < _slotCount; i++)
    {
        if (_slots[i].occupied &&
            _slots[i].uid == cleanUID)
        {
            return static_cast<int8_t>(i);
        }
    }

    return INVALID_SLOT;
}

int8_t SlotManager::findSlotByCity(
    const String& city
) const
{
    if (!_ready)
    {
        return INVALID_SLOT;
    }

    String cleanCity = city;
    cleanCity.trim();

    if (cleanCity.isEmpty())
    {
        return INVALID_SLOT;
    }

    for (uint8_t i = 0; i < _slotCount; i++)
    {
        if (_slots[i].occupied &&
            _slots[i].city.equalsIgnoreCase(cleanCity))
        {
            return static_cast<int8_t>(i);
        }
    }

    return INVALID_SLOT;
}

bool SlotManager::isOccupied(
    uint8_t slotIndex
) const
{
    if (!_ready || !validIndex(slotIndex))
    {
        return false;
    }

    return _slots[slotIndex].occupied;
}

bool SlotManager::isUIDStored(
    const String& uid
) const
{
    return findSlotByUID(uid) != INVALID_SLOT;
}

bool SlotManager::full() const
{
    return _ready &&
           _occupiedCount >= _slotCount;
}

bool SlotManager::isEmpty() const
{
    return _ready &&
           _occupiedCount == 0;
}

bool SlotManager::ready() const
{
    return _ready;
}

uint8_t SlotManager::slotCount() const
{
    return _slotCount;
}

uint8_t SlotManager::occupiedCount() const
{
    return _occupiedCount;
}

int8_t SlotManager::lastChangedSlot() const
{
    return _lastChangedSlot;
}

const SlotInfo* SlotManager::slot(
    uint8_t slotIndex
) const
{
    if (!_ready || !validIndex(slotIndex))
    {
        return nullptr;
    }

    return &_slots[slotIndex];
}

bool SlotManager::validIndex(
    uint8_t slotIndex
) const
{
    return slotIndex < _slotCount;
}

String SlotManager::normalizeUID(
    const String& uid
)
{
    String cleanUID = uid;

    cleanUID.trim();
    cleanUID.toUpperCase();

    return cleanUID;
}