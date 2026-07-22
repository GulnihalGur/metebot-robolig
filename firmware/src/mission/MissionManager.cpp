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

    // Tam sifirlama yalnizca acilista yapilir.
    resetMissionData();
}

void MissionManager::update()
{
    // Disaridan aktif edilen FailSafe'i yakalar.
    if (failSafe.isActive() &&
        !stateMachine.isState(RobotState::FAIL_SAFE))
    {
        stateBeforeFailSafe =
            stateMachine.getCurrentState();

        recoveryStateAvailable = true;

        changeState(RobotState::FAIL_SAFE);
    }

    updateState(stateMachine.getCurrentState());
}

bool MissionManager::requestState(
    RobotState newState
)
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
    City city,
    int8_t preferredSlot
)
{
    if (!stateMachine.isState(RobotState::PICKUP))
    {
        return false;
    }

    if (!rfid.ready())
    {
        rfidReadStatus =
            RFIDReadStatus::HARDWARE_ERROR;

        return false;
    }

    if (!slotManager.ready() ||
        slotManager.full())
    {
        return false;
    }

    // Gecersiz sehir kaydedilemez.
    if (city == City::NONE)
    {
        return false;
    }

    String uid;

    rfidReadStatus = rfid.readUIDString(
        uid,
        RFID::DEFAULT_READ_TIMEOUT_MS
    );

    if (rfidReadStatus !=
        RFIDReadStatus::READ_SUCCESS)
    {
        return false;
    }

    // Ayni UID ikinci kez eklenemez.
    if (slotManager.isUIDStored(uid))
    {
        return false;
    }

    return slotManager.placeItem(
        uid,
        city,
        preferredSlot
    );
}

bool MissionManager::confirmPickupCompleted()
{
    if (!stateMachine.isState(RobotState::PICKUP))
    {
        return false;
    }

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
    if (!stateMachine.isState(
            RobotState::DELIVERY))
    {
        return SlotManager::INVALID_SLOT;
    }

    if (!rfid.ready())
    {
        rfidReadStatus =
            RFIDReadStatus::HARDWARE_ERROR;

        return SlotManager::INVALID_SLOT;
    }

    if (!slotManager.ready())
    {
        return SlotManager::INVALID_SLOT;
    }

    String uid;

    rfidReadStatus = rfid.readUIDString(
        uid,
        RFID::DEFAULT_READ_TIMEOUT_MS
    );

    if (rfidReadStatus !=
        RFIDReadStatus::READ_SUCCESS)
    {
        return SlotManager::INVALID_SLOT;
    }

    // Kayit silinmeden ilgili slot bulunur.
    return slotManager.findSlotByUID(uid);
}

RFIDReadStatus
MissionManager::lastRfidReadStatus() const
{
    return rfidReadStatus;
}

bool MissionManager::confirmDeliveryItemReleased(
    uint8_t slotIndex
)
{
    if (!stateMachine.isState(
            RobotState::DELIVERY))
    {
        return false;
    }

    if (!slotManager.ready())
    {
        return false;
    }

    // Bos veya gecersiz slot teslim edilemez.
    if (!slotManager.isOccupied(slotIndex))
    {
        return false;
    }

    return slotManager.removeItem(slotIndex);
}

bool MissionManager::confirmDeliveryCompleted()
{
    if (!stateMachine.isState(
            RobotState::DELIVERY))
    {
        return false;
    }

    if (!slotManager.isEmpty())
    {
        return false;
    }

    return requestState(RobotState::ZIPLINE);
}

// =====================================================
// ZIPLINE ONAYLARI
// =====================================================

void MissionManager::
confirmZiplineExtensionCompleted()
{
    if (stateMachine.isState(RobotState::ZIPLINE))
    {
        zipline.confirmExtensionCompleted();
    }
}

void MissionManager::
confirmZiplinePositioningCompleted()
{
    if (stateMachine.isState(RobotState::ZIPLINE))
    {
        zipline.confirmPositioningCompleted();
    }
}

void MissionManager::
confirmZiplineSlideCompleted()
{
    if (stateMachine.isState(RobotState::ZIPLINE))
    {
        zipline.confirmSlideCompleted();
    }
}

void MissionManager::
confirmZiplineRetractionCompleted()
{
    if (stateMachine.isState(RobotState::ZIPLINE))
    {
        zipline.confirmRetractionCompleted();
    }
}

bool MissionManager::confirmCurrentZiplineStep()
{
    if (!stateMachine.isState(RobotState::ZIPLINE))
    {
        return false;
    }

    // Tek tus basisi yalnizca bir adim ilerletir.
    switch (zipline.getState())
    {
        case Zipline::State::EXTENDING:
            zipline.confirmExtensionCompleted();

            return zipline.getState() ==
                   Zipline::State::POSITIONING;

        case Zipline::State::POSITIONING:
            zipline.confirmPositioningCompleted();

            return zipline.getState() ==
                   Zipline::State::SLIDING;

        case Zipline::State::SLIDING:
            zipline.confirmSlideCompleted();

            return zipline.getState() ==
                   Zipline::State::RETRACTING;

        case Zipline::State::RETRACTING:
            zipline.confirmRetractionCompleted();

            return zipline.getState() ==
                   Zipline::State::COMPLETED;

        default:
            return false;
    }
}

Zipline::State
MissionManager::currentZiplineState() const
{
    return zipline.getState();
}

// =====================================================
// SAFETY / RECOVERY
// =====================================================

void MissionManager::activateFailSafe()
{
    if (!stateMachine.isState(
            RobotState::FAIL_SAFE))
    {
        stateBeforeFailSafe =
            stateMachine.getCurrentState();

        recoveryStateAvailable = true;
    }

    failSafe.activate();

    if (!stateMachine.isState(
            RobotState::FAIL_SAFE))
    {
        changeState(RobotState::FAIL_SAFE);
    }
}

bool MissionManager::recoverFromFailSafe()
{
    if (!failSafe.isActive() ||
        !stateMachine.isState(
            RobotState::FAIL_SAFE))
    {
        return false;
    }

    // Kurtarma sirasinda hareket kapatilir.
    motionController.stop();
    linearActuator.stop();

    RobotState recoveryState =
        RobotState::IDLE;

    if (recoveryStateAvailable)
    {
        recoveryState = stateBeforeFailSafe;
    }

    // Zipline otomatik devam ettirilmez.
    if (recoveryState == RobotState::ZIPLINE)
    {
        zipline.reset();
        recoveryState = RobotState::DELIVERY;
    }

    if (recoveryState == RobotState::FAIL_SAFE ||
        recoveryState == RobotState::FINISH)
    {
        recoveryState = RobotState::IDLE;
    }

    failSafe.deactivate();
    motionController.clearEmergencyStop();

    recoveryStateAvailable = false;
    stateBeforeFailSafe = RobotState::IDLE;

    changeState(recoveryState);

    return true;
}

void MissionManager::resetMissionData()
{
    failSafe.deactivate();

    motionController.clearEmergencyStop();
    motionController.stop();
    linearActuator.stop();

    // Tum gorev verileri acilista temizlenir.
    slotManager.clearAll();
    zipline.reset();
    finish.reset();

    recoveryStateAvailable = false;
    stateBeforeFailSafe = RobotState::IDLE;
    rfidReadStatus = RFIDReadStatus::NO_TAG;

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
        case RobotState::PICKUP:
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
    // Her durumdan FailSafe'e gecilebilir.
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
            // Cikis sadece recovery ile yapilir.
            return false;
    }

    return false;
}

void MissionManager::changeState(
    RobotState newState
)
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