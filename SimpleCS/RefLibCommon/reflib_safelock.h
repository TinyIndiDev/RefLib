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
            _crit.Lock();
        }
        ~Owner()
        {
            _crit.Unlock();
        }

    private:
        SafeLock &_crit;
    };

    SafeLock();
    ~SafeLock();

private:
    void Lock();
    void Unlock();

    CRITICAL_SECTION _crit;
};

} //namespace RefLib
