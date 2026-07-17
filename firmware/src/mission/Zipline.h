#pragma once

#include <control/MotionController.h>
#include <drivers/LinearActuator.h>
#include "../core/FailSafe.h"

class Zipline
{
public:
    enum class State
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
        MotionController& motionController,
        LinearActuator& linearActuator,
        FailSafe& failSafe
    );

    void begin();
    void start();
    void update();

    void confirmExtensionCompleted();
    void confirmPositioningCompleted();
    void confirmSlideCompleted();
    void confirmRetractionCompleted();

    void reset();
    void cancel();
    void fail();

    bool isActive() const;
    bool isCompleted() const;
    bool hasFailed() const;

    State getState() const;

private:
    MotionController& motionController;
    LinearActuator& linearActuator;
    FailSafe& failSafe;

    State currentState = State::IDLE;

    bool canStartSliding() const;
};