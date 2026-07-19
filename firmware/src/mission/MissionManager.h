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

    // Görev sistemini başlatır.
    void begin();

    // loop() içerisinde sürekli çağrılır.
    void update();

    /*
     * Robotun görev durumunu değiştirmek için kullanılır.
     * Geçiş izinli değilse false döndürür.
     */
    bool requestState(RobotState newState);


    // =====================================================
    // PICKUP
    // =====================================================

    /*
     * RFID ile koli UID'sini okur ve şehir bilgisiyle
     * uygun slota kaydeder.
     *
     * preferredSlot verilmezse ilk boş slot kullanılır.
     */
    bool scanPickupItem(
        const String& city,
        int8_t preferredSlot = SlotManager::INVALID_SLOT
    );

    /*
     * Bütün slotlar doluysa PICKUP görevini tamamlar
     * ve DELIVERY durumuna geçer.
     */
    bool confirmPickupCompleted();


    // =====================================================
    // DELIVERY
    // =====================================================

    /*
     * RFID ile koli okur ve kolinin bulunduğu slotu döndürür.
     * Slot kaydı bu fonksiyon içinde silinmez.
     */
    int8_t scanDeliveryItem();

    /*
     * Pilot koliyi fiziksel olarak bıraktıktan sonra
     * ilgili slot kaydını siler.
     */
    bool confirmDeliveryItemReleased(uint8_t slotIndex);

    /*
     * Bütün slotlar boşsa DELIVERY görevini tamamlar
     * ve ZIPLINE durumuna geçer.
     */
    bool confirmDeliveryCompleted();


    // =====================================================
    // ZIPLINE PILOT ONAYLARI
    // =====================================================

    void confirmZiplineExtensionCompleted();
    void confirmZiplinePositioningCompleted();
    void confirmZiplineSlideCompleted();
    void confirmZiplineRetractionCompleted();

    /*
     * Tek pilot onay tusuyla mevcut zipline alt adimini
     * yalnizca bir kez ilerletir.
     */
    bool confirmCurrentZiplineStep();

    // Mevcut zipline alt durumunu dondurur.
    Zipline::State currentZiplineState() const;


    // =====================================================
    // SAFETY / RECOVERY
    // =====================================================

    // Robotu FailSafe durumuna geçirir.
    void activateFailSafe();

    /*
     * FailSafe'den görev ve slot verilerini silmeden çıkar.
     *
     * Robot FailSafe durumunda değilse false döndürür.
     */
    bool recoverFromFailSafe();

    // Mevcut ana görev durumunu döndürür.
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

    /*
     * Yalnızca sistem ilk başlatılırken kullanılır.
     * UART veya dış modüller tarafından çağrılamaz.
     */
    void resetMissionData();

    /*
     * FailSafe öncesindeki görev durumunu saklar.
     */
    RobotState stateBeforeFailSafe = RobotState::IDLE;

    /*
     * Geri dönülebilecek geçerli bir görev durumu
     * saklanıp saklanmadığını belirtir.
     */
    bool recoveryStateAvailable = false;

    void enterState(RobotState state);
    void updateState(RobotState state);
    void exitState(RobotState state);

    bool canTransition(
        RobotState from,
        RobotState to
    ) const;

    void changeState(RobotState newState);
};
