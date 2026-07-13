#include "Zipline.h"

Zipline::Zipline(
    MotorDriver& motorDriver,
    LinearActuator& linearActuator,
    FailSafe& failSafe
)
    : motorDriver(motorDriver),
      linearActuator(linearActuator),
      failSafe(failSafe)
{
}

void Zipline::begin()
{
    currentState = State::IDLE;
}

void Zipline::start()
{
    if (isActive() || isCompleted())
    {
        return;
    }

    if (failSafe.isActive() ||
        !motorDriver.ready() ||
        !linearActuator.ready())
    {
        fail();
        return;
    }

    motorDriver.stopDrive();
    linearActuator.extend();

    currentState = State::EXTENDING;
}

void Zipline::update()
{
    // Süreli aktüatör hareketlerinin otomatik durmasını sağlar.
    linearActuator.update();

    if (!isActive())
    {
        return;
    }

    if (failSafe.isActive())
    {
        motorDriver.stopAll();
        linearActuator.stop();

        currentState = State::FAILED;
        return;
    }

    switch (currentState)
    {
        case State::EXTENDING:
            // Aktüatör uzar; yeterli konum pilot tarafından onaylanır.
            break;

        case State::POSITIONING:
            /*
             * Pilot, joystick kullanarak robotu zipline hattına yerleştirir.
             * Sürüş motorları bu aşamada manuel olarak kontrol edilebilir.
             */
            break;

        case State::SLIDING:
            /*
             * Ayrı bir zipline motoru bulunmamaktadır.
             * Sürüş motorları ve aktüatör durdurulur;
             * robot eğim ve yerçekimiyle serbestçe kayar.
             */
            motorDriver.stopDrive();
            linearActuator.stop();
            break;

        case State::RETRACTING:
            // Aktüatör geri çekilir; bitiş pilot tarafından onaylanır.
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

void Zipline::confirmPositioning()
{
    if (currentState != State::POSITIONING)
    {
        return;
    }

    motorDriver.stopDrive();
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

    motorDriver.stopAll();
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
    motorDriver.stopDrive();
    linearActuator.stop();

    currentState = State::IDLE;
}

void Zipline::cancel()
{
    motorDriver.stopAll();
    linearActuator.stop();

    currentState = State::CANCELLED;
}

void Zipline::fail()
{
    motorDriver.stopAll();
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
           motorDriver.ready() &&
           linearActuator.ready() &&
           linearActuator.direction() ==
               LinearActuator::Direction::Stop;
}