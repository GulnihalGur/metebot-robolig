#ifndef FAILSAFE_H
#define FAILSAFE_H

class FailSafe
{
public:
    void activate();
    void deactivate();

    bool isActive() const;

private:
    bool active = false;
};

#endif