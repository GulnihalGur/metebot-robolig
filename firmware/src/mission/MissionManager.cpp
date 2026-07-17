#include "MissionManager.h"


MissionManager::MissionManager(
    StateMachine& stateMachine,
    FailSafe& failSafe,
    MotionController& motionController,
    SlotManager& slotManager,
    RFID& rfid,
    LinearActuator& linearActuator,
    Zipline& zipline,
    Finish& finish
)
    : stateMachine(stateMachine),
      failSafe(failSafe),
      motionController(motionController),
      slotManager(slotManager),
      rfid(rfid),
      linearActuator(linearActuator),
      zipline(zipline),
      finish(finish)
{
}


void MissionManager::begin()
{
    stateMachine.begin();
    zipline.begin();

    /*
     * Tam görev sıfırlaması yalnızca sistem açılışında yapılır.
     */
    resetMissionData();
}


void MissionManager::update()
{
    /*
     * FailSafe başka bir modül tarafından doğrudan aktif edilmişse
     * robotun mevcut görev durumu kurtarma için saklanır.
     */
    if (failSafe.isActive() &&
        !stateMachine.isState(RobotState::FAIL_SAFE))
    {
        stateBeforeFailSafe = stateMachine.getCurrentState();
        recoveryStateAvailable = true;

        changeState(RobotState::FAIL_SAFE);
    }

    updateState(stateMachine.getCurrentState());
}


bool MissionManager::requestState(RobotState newState)
{
    const RobotState current =
        stateMachine.getCurrentState();

    if (current == newState)
    {
        return true;
    }

    if (!canTransition(current, newState))
    {
        return false;
    }

    changeState(newState);

    return true;
}


// =====================================================
// PICKUP
// =====================================================

bool MissionManager::scanPickupItem(
    const String& city,
    int8_t preferredSlot
)
{
    if (!stateMachine.isState(RobotState::PICKUP))
    {
        return false;
    }

    if (!rfid.ready() ||
        !slotManager.ready() ||
        slotManager.full())
    {
        return false;
    }

    String cleanCity = city;
    cleanCity.trim();

    if (cleanCity.length() == 0)
    {
        return false;
    }

    const String uid = rfid.readUIDString();

    if (uid.length() == 0)
    {
        return false;
    }

    /*
     * Aynı RFID UID ikinci kez kaydedilemez.
     */
    if (slotManager.isUIDStored(uid))
    {
        return false;
    }

    return slotManager.placeItem(
        uid,
        cleanCity,
        preferredSlot
    );
}


bool MissionManager::confirmPickupCompleted()
{
    if (!stateMachine.isState(RobotState::PICKUP))
    {
        return false;
    }

    /*
     * Pickup görevi yalnızca bütün slotlar doluysa tamamlanır.
     */
    if (!slotManager.full())
    {
        return false;
    }

    return requestState(RobotState::DELIVERY);
}


// =====================================================
// DELIVERY
// =====================================================

int8_t MissionManager::scanDeliveryItem()
{
    if (!stateMachine.isState(RobotState::DELIVERY))
    {
        return SlotManager::INVALID_SLOT;
    }

    if (!rfid.ready() || !slotManager.ready())
    {
        return SlotManager::INVALID_SLOT;
    }

    const String uid = rfid.readUIDString();

    if (uid.length() == 0)
    {
        return SlotManager::INVALID_SLOT;
    }

    /*
     * RFID ile okunan kolinin bulunduğu slot döndürülür.
     * Bu aşamada herhangi bir slot kaydı silinmez.
     */
    return slotManager.findSlotByUID(uid);
}


bool MissionManager::confirmDeliveryItemReleased(
    uint8_t slotIndex
)
{
    if (!stateMachine.isState(RobotState::DELIVERY))
    {
        return false;
    }

    if (!slotManager.ready())
    {
        return false;
    }

    /*
     * Geçersiz veya boş slot teslim edilmiş sayılamaz.
     */
    if (!slotManager.isOccupied(slotIndex))
    {
        return false;
    }

    /*
     * Pilot fiziksel teslimatı onayladıktan sonra
     * yalnızca ilgili slotun kaydı silinir.
     */
    return slotManager.removeItem(slotIndex);
}


bool MissionManager::confirmDeliveryCompleted()
{
    if (!stateMachine.isState(RobotState::DELIVERY))
    {
        return false;
    }

    /*
     * Slotlarda koli kalmışsa DELIVERY tamamlanamaz.
     */
    if (!slotManager.isEmpty())
    {
        return false;
    }

    return requestState(RobotState::ZIPLINE);
}


// =====================================================
// ZIPLINE PILOT ONAYLARI
// =====================================================

void MissionManager::confirmZiplineExtensionCompleted()
{
    if (stateMachine.isState(RobotState::ZIPLINE))
    {
        zipline.confirmExtensionCompleted();
    }
}


void MissionManager::confirmZiplinePositioningCompleted()
{
    if (stateMachine.isState(RobotState::ZIPLINE))
    {
        zipline.confirmPositioningCompleted();
    }
}


void MissionManager::confirmZiplineSlideCompleted()
{
    if (stateMachine.isState(RobotState::ZIPLINE))
    {
        zipline.confirmSlideCompleted();
    }
}


void MissionManager::confirmZiplineRetractionCompleted()
{
    if (stateMachine.isState(RobotState::ZIPLINE))
    {
        zipline.confirmRetractionCompleted();
    }
}


// =====================================================
// SAFETY / RECOVERY
// =====================================================

void MissionManager::activateFailSafe()
{
    /*
     * FailSafe'e ilk kez girerken robotun mevcut görev durumu
     * saklanır. Robot zaten FAIL_SAFE durumundaysa kayıt ezilmez.
     */
    if (!stateMachine.isState(RobotState::FAIL_SAFE))
    {
        stateBeforeFailSafe = stateMachine.getCurrentState();
        recoveryStateAvailable = true;
    }

    failSafe.activate();

    if (!stateMachine.isState(RobotState::FAIL_SAFE))
    {
        changeState(RobotState::FAIL_SAFE);
    }
}


bool MissionManager::recoverFromFailSafe()
{
    /*
     * Robot FailSafe durumunda değilse kurtarma yapılamaz.
     */
    if (!failSafe.isActive() ||
        !stateMachine.isState(RobotState::FAIL_SAFE))
    {
        return false;
    }

    /*
     * Kurtarma sırasında robot kesinlikle hareket etmemelidir.
     */
    motionController.stop();
    linearActuator.stop();

    /*
     * Varsayılan güvenli dönüş durumu IDLE'dır.
     */
    RobotState recoveryState = RobotState::IDLE;

    if (recoveryStateAvailable)
    {
        recoveryState = stateBeforeFailSafe;
    }

    /*
     * Zipline sırasında FailSafe oluşursa zipline otomatik
     * olarak devam ettirilmez.
     *
     * Robot DELIVERY aşamasına döndürülür ve pilotun
     * zipline geçişini yeniden başlatması beklenir.
     */
    if (recoveryState == RobotState::ZIPLINE)
    {
        zipline.reset();
        recoveryState = RobotState::DELIVERY;
    }

    /*
     * FAIL_SAFE veya FINISH durumuna otomatik dönüş yapılmaz.
     */
    if (recoveryState == RobotState::FAIL_SAFE ||
        recoveryState == RobotState::FINISH)
    {
        recoveryState = RobotState::IDLE;
    }

    /*
     * FailSafe bayrağı kaldırılır.
     * Slot ve RFID görev verileri kesinlikle silinmez.
     */
    failSafe.deactivate();
    motionController.clearEmergencyStop();

    recoveryStateAvailable = false;
    stateBeforeFailSafe = RobotState::IDLE;

    changeState(recoveryState);

    return true;
}


void MissionManager::resetMissionData()
{
    /*
     * Bu fonksiyon yalnızca robot ilk açılırken çağrılır.
     * Yarış sırasında veya UART komutuyla çağrılamaz.
     */
    failSafe.deactivate();

    motionController.clearEmergencyStop();
    motionController.stop();
    linearActuator.stop();

    /*
     * Tam görev temizliği yalnızca burada yapılır.
     */
    slotManager.clearAll();
    zipline.reset();
    finish.reset();

    recoveryStateAvailable = false;
    stateBeforeFailSafe = RobotState::IDLE;

    changeState(RobotState::IDLE);
}


RobotState MissionManager::currentState() const
{
    return stateMachine.getCurrentState();
}


// =====================================================
// STATE MANAGEMENT
// =====================================================

void MissionManager::enterState(RobotState state)
{
    switch (state)
    {
        case RobotState::IDLE:
            motionController.stop();
            linearActuator.stop();
            break;

        case RobotState::MANUAL:
            if (!failSafe.isActive())
            {
                motionController.clearEmergencyStop();
            }
            break;

        case RobotState::PICKUP:
            if (!failSafe.isActive())
            {
                motionController.clearEmergencyStop();
            }
            break;

        case RobotState::DELIVERY:
            if (!failSafe.isActive())
            {
                motionController.clearEmergencyStop();
            }
            break;

        case RobotState::ZIPLINE:
            motionController.stop();
            zipline.start();
            break;

        case RobotState::FINISH:
            finish.enter();
            break;

        case RobotState::FAIL_SAFE:
            motionController.emergencyStop();
            linearActuator.stop();
            break;
    }
}


void MissionManager::updateState(RobotState state)
{
    switch (state)
    {
        case RobotState::IDLE:
            motionController.stop();
            linearActuator.stop();
            break;

        case RobotState::MANUAL:
        case RobotState::PICKUP:
        case RobotState::DELIVERY:
            /*
             * Joystick/UART katmanı sürüş hedeflerini verir.
             * MotionController motor çıkışlarını günceller.
             */
            motionController.update();
            break;

        case RobotState::ZIPLINE:
            zipline.update();

            if (zipline.isCompleted())
            {
                changeState(RobotState::FINISH);
            }
            else if (zipline.hasFailed())
            {
                activateFailSafe();
            }
            break;

        case RobotState::FINISH:
            finish.update();
            break;

        case RobotState::FAIL_SAFE:
            motionController.emergencyStop();
            linearActuator.stop();
            break;
    }
}


void MissionManager::exitState(RobotState state)
{
    switch (state)
    {
        case RobotState::IDLE:
            break;

        case RobotState::MANUAL:
        case RobotState::PICKUP:
        case RobotState::DELIVERY:
            motionController.stop();
            break;

        case RobotState::ZIPLINE:
            /*
             * Zipline başarıyla tamamlandıysa tamamlanma durumu korunur.
             * Başka nedenle çıkılıyorsa güvenli biçimde iptal edilir.
             */
            if (!zipline.isCompleted())
            {
                zipline.cancel();
            }
            break;

        case RobotState::FINISH:
            finish.exit();
            break;

        case RobotState::FAIL_SAFE:
            break;
    }
}


bool MissionManager::canTransition(
    RobotState from,
    RobotState to
) const
{
    /*
     * Her görev durumundan FAIL_SAFE'e geçilebilir.
     */
    if (to == RobotState::FAIL_SAFE)
    {
        return true;
    }

    switch (from)
    {
        case RobotState::IDLE:
            return to == RobotState::MANUAL ||
                   to == RobotState::PICKUP;

        case RobotState::MANUAL:
            return to == RobotState::IDLE ||
                   to == RobotState::PICKUP;

        case RobotState::PICKUP:
            return to == RobotState::IDLE ||
                   to == RobotState::MANUAL ||
                   to == RobotState::DELIVERY;

        case RobotState::DELIVERY:
            return to == RobotState::IDLE ||
                   to == RobotState::MANUAL ||
                   to == RobotState::ZIPLINE;

        case RobotState::ZIPLINE:
            return to == RobotState::FINISH;

        case RobotState::FINISH:
            return to == RobotState::IDLE;

        case RobotState::FAIL_SAFE:
            /*
             * Normal requestState() ile FailSafe'den çıkılmaz.
             * FailSafe çıkışı yalnızca recoverFromFailSafe()
             * üzerinden yapılır.
             */
            return false;
    }

    return false;
}


void MissionManager::changeState(RobotState newState)
{
    const RobotState oldState =
        stateMachine.getCurrentState();

    if (oldState == newState)
    {
        return;
    }

    exitState(oldState);

    stateMachine.changeState(newState);

    enterState(newState);

    stateMachine.clearStateChangedFlag();
}