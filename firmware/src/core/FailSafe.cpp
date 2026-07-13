#include "FailSafe.h"

void FailSafe::activate()
{
    active = true;
}

void FailSafe::deactivate()
{
    active = false;
}

bool FailSafe::isActive() const
{
    return active;
}