#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include "Types.h"

class StateMachine
{
public:
    void begin();

    void changeState(RobotState newState);

    RobotState getCurrentState() const;
    RobotState getPreviousState() const;

    bool isState(RobotState state) const;
    bool hasStateChanged() const;

    void clearStateChangedFlag();

private:
    RobotState currentState = RobotState::IDLE;
    RobotState previousState = RobotState::IDLE;

    bool stateChanged = false;
};

#endif