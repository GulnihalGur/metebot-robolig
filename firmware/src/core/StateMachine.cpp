#include "StateMachine.h"

void StateMachine::begin()
{
    currentState = RobotState::IDLE;
    previousState = RobotState::IDLE;
    stateChanged = false;
}

void StateMachine::changeState(RobotState newState)
{
    if (currentState != newState)
    {
        previousState = currentState;
        currentState = newState;
        stateChanged = true;
    }
}

RobotState StateMachine::getCurrentState() const
{
    return currentState;
}

RobotState StateMachine::getPreviousState() const
{
    return previousState;
}

bool StateMachine::isState(RobotState state) const
{
    return currentState == state;
}
 
bool StateMachine::hasStateChanged() const
{
    return stateChanged;
}

void StateMachine::clearStateChangedFlag()
{
    stateChanged = false;
}