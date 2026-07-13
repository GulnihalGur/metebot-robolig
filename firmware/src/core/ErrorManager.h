#ifndef ERRORMANAGER_H
#define ERRORMANAGER_H

#include "Types.h"

class ErrorManager
{
public:
    void clearError();
    void setError(ErrorCode code);
    bool hasError() const;
    ErrorCode getCurrentError() const;

private:
    ErrorCode currentError = ErrorCode::NONE;
};

#endif