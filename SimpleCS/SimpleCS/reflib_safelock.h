#pragma once

#include "reflib_non_copyable.h"

namespace RefLib
{

class SafeLock : public NonCopyable
{
public:
    class Owner : public NonCopyable
    {
    public:
        explicit Owner(SafeLock &crit) : _crit(crit)
        {
            _crit.Enter();
        }
        ~Owner()
        {
            _crit.Leave();
        }

    private:
        SafeLock &_crit;
    };

    SafeLock();
    ~SafeLock();

private:
    void Enter();
    void Leave();

    CRITICAL_SECTION _crit;
};

} //namespace RefLib
