#ifndef ZIPLINE_H
#define ZIPLINE_H

#include <Arduino.h>

#include "../core/FailSafe.h"
#include "../drivers/MotorDriver.h"
#include "../drivers/LinearActuator.h"

class Zipline
{
public:
    enum class State : uint8_t
    {
        IDLE,
        EXTENDING,
        POSITIONING,
        SLIDING,
        RETRACTING,
        COMPLETED,
        CANCELLED,
        FAILED
    };

    Zipline(
        MotorDriver& motorDriver,
        LinearActuator& linearActuator,
        FailSafe& failSafe
    );

    void begin();
    void start();
    void update();
    void reset();

    // Pilot, aktüatörün yeterince uzadığını onaylar.
    void confirmExtensionCompleted();

    // Pilot, robotun hatta doğru yerleştiğini onaylar.
    void confirmPositioning();

    // Pilot, robotun karşı platforma geçtiğini onaylar.
    void confirmSlideCompleted();

    // Pilot, aktüatörün geri çekilmesinin tamamlandığını onaylar.
    void confirmRetractionCompleted();

    void cancel();
    void fail();

    bool isActive() const;
    bool isCompleted() const;
    bool hasFailed() const;

    State getState() const;

private:
    MotorDriver& motorDriver;
    LinearActuator& linearActuator;
    FailSafe& failSafe;

    State currentState = State::IDLE;

    bool canStartSliding() const;
};

#endif