#pragma once

#include <Arduino.h>
#include "Types.h"

#include "../core/StateMachine.h"
#include "../core/FailSafe.h"

#include "../control/MotionController.h"
#include "../control/SlotManager.h"

#include "../drivers/RFID.h"
#include "../drivers/LinearActuator.h"

#include "Zipline.h"
#include "Finish.h"

class MissionManager
{
public:
    MissionManager(
        StateMachine& stateMachine,
        FailSafe& failSafe,
        MotionController& motionController,
        SlotManager& slotManager,
        RFID& rfid,
        LinearActuator& linearActuator,
        Zipline& zipline,
        Finish& finish
    );

    // Gorev sistemini baslatir.
    void begin();

    // Ana dongude surekli cagrilir.
    void update();

    // Ana gorev durumunu degistirir.
    bool requestState(RobotState newState);

    // =====================================================
    // PICKUP
    // =====================================================

    // RFID ile okunan yuku slota kaydeder.
    bool scanPickupItem(
        City city,
        int8_t preferredSlot =
            SlotManager::INVALID_SLOT
    );

    // Slotlar doluysa DELIVERY durumuna gecer.
    bool confirmPickupCompleted();

    // =====================================================
    // DELIVERY
    // =====================================================

    // Okunan UID'nin bulundugu slotu dondurur.
    int8_t scanDeliveryItem();

    // Son RFID okuma sonucunu dondurur.
    RFIDReadStatus lastRfidReadStatus() const;

    // Fiziksel teslimattan sonra slotu temizler.
    bool confirmDeliveryItemReleased(
        uint8_t slotIndex
    );

    // Slotlar bossa ZIPLINE durumuna gecer.
    bool confirmDeliveryCompleted();

    // =====================================================
    // ZIPLINE ONAYLARI
    // =====================================================

    void confirmZiplineExtensionCompleted();
    void confirmZiplinePositioningCompleted();
    void confirmZiplineSlideCompleted();
    void confirmZiplineRetractionCompleted();

    // Mevcut zipline adimini bir kez ilerletir.
    bool confirmCurrentZiplineStep();

    // Mevcut zipline alt durumunu dondurur.
    Zipline::State currentZiplineState() const;

    // =====================================================
    // SAFETY / RECOVERY
    // =====================================================

    // Robotu FailSafe durumuna gecirir.
    void activateFailSafe();

    // Gorev verilerini silmeden kurtarma yapar.
    bool recoverFromFailSafe();

    // Ana gorev durumunu dondurur.
    RobotState currentState() const;

private:
    StateMachine& stateMachine;
    FailSafe& failSafe;

    MotionController& motionController;
    SlotManager& slotManager;

    RFID& rfid;
    LinearActuator& linearActuator;

    Zipline& zipline;
    Finish& finish;

    // Yalnizca sistem acilisinda tum gorevi temizler.
    void resetMissionData();

    RobotState stateBeforeFailSafe =
        RobotState::IDLE;

    bool recoveryStateAvailable = false;

    RFIDReadStatus rfidReadStatus =
        RFIDReadStatus::NO_TAG;

    void enterState(RobotState state);
    void updateState(RobotState state);
    void exitState(RobotState state);

    bool canTransition(
        RobotState from,
        RobotState to
    ) const;

    void changeState(RobotState newState);
};