#include "ErrorManager.h"

void ErrorManager::setError(ErrorCode errorCode)
{
    currentError = errorCode;
}

void ErrorManager::clearError()
{
    currentError = ErrorCode::NONE;
}

bool ErrorManager::hasError() const
{
    return currentError != ErrorCode::NONE;
}

ErrorCode ErrorManager::getCurrentError() const
{
    return currentError;
}