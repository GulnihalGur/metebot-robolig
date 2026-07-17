#include "Zipline.h"

Zipline::Zipline(
    MotionController& motionController,
    LinearActuator& linearActuator,
    FailSafe& failSafe
)
    : motionController(motionController),
      linearActuator(linearActuator),
      failSafe(failSafe)
{
}

void Zipline::begin()
{
    reset();
}

void Zipline::start()
{
    if (isActive() || isCompleted())
    {
        return;
    }

    if (failSafe.isActive() ||
        !motionController.ready() ||
        !linearActuator.ready())
    {
        fail();
        return;
    }

    motionController.stop();
    linearActuator.extend();

    currentState = State::EXTENDING;
}

void Zipline::update()
{
    linearActuator.update();
    motionController.update();

    if (!isActive())
    {
        return;
    }

    if (failSafe.isActive())
    {
        fail();
        return;
    }

    switch (currentState)
    {
        case State::EXTENDING:
            /*
             * Lineer aktüatör uzar.
             * Limit switch olmadığı için pilot uzama tamamlandığında
             * confirmExtensionCompleted() çağrısını yapar.
             */
            break;

        case State::POSITIONING:
            /*
             * Pilot robotu joystick ile zipline hattına hizalar.
             * Bu aşamada MotionController üzerinden manuel sürüş yapılabilir.
             */
            break;

        case State::SLIDING:
            /*
             * Zipline için ayrı motor yoktur.
             * Robot eğim ve yerçekimiyle kayar.
             */
            motionController.stop();
            linearActuator.stop();
            break;

        case State::RETRACTING:
            /*
             * Aktüatör geri çekilir.
             * Limit switch olmadığı için pilot tamamlandığını onaylar.
             */
            break;

        default:
            break;
    }
}

void Zipline::confirmExtensionCompleted()
{
    if (currentState != State::EXTENDING)
    {
        return;
    }

    linearActuator.stop();
    currentState = State::POSITIONING;
}

void Zipline::confirmPositioningCompleted()
{
    if (currentState != State::POSITIONING)
    {
        return;
    }

    motionController.stop();
    linearActuator.stop();

    if (!canStartSliding())
    {
        fail();
        return;
    }

    currentState = State::SLIDING;
}

void Zipline::confirmSlideCompleted()
{
    if (currentState != State::SLIDING)
    {
        return;
    }

    motionController.stop();
    linearActuator.retract();

    currentState = State::RETRACTING;
}

void Zipline::confirmRetractionCompleted()
{
    if (currentState != State::RETRACTING)
    {
        return;
    }

    linearActuator.stop();
    currentState = State::COMPLETED;
}

void Zipline::reset()
{
    motionController.stop();
    linearActuator.stop();

    currentState = State::IDLE;
}

void Zipline::cancel()
{
    motionController.stop();
    linearActuator.stop();

    currentState = State::CANCELLED;
}

void Zipline::fail()
{
    motionController.emergencyStop();
    linearActuator.stop();

    currentState = State::FAILED;
}

bool Zipline::isActive() const
{
    return currentState == State::EXTENDING ||
           currentState == State::POSITIONING ||
           currentState == State::SLIDING ||
           currentState == State::RETRACTING;
}

bool Zipline::isCompleted() const
{
    return currentState == State::COMPLETED;
}

bool Zipline::hasFailed() const
{
    return currentState == State::FAILED;
}

Zipline::State Zipline::getState() const
{
    return currentState;
}

bool Zipline::canStartSliding() const
{
    return !failSafe.isActive() &&
           motionController.ready() &&
           !motionController.emergencyStopped() &&
           linearActuator.ready() &&
           linearActuator.direction() ==
               LinearActuator::Direction::Stop;
}