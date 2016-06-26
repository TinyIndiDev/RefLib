#include "stdafx.h"

#include "reflib_safelock.h"

namespace RefLib
{

SafeLock::SafeLock()
{
    ::InitializeCriticalSection(&_crit);
}

SafeLock::~SafeLock()
{
    ::DeleteCriticalSection(&_crit);
}

void SafeLock::Lock()
{
    ::EnterCriticalSection(&_crit);
}

void SafeLock::Unlock()
{
    ::LeaveCriticalSection(&_crit);
}

} // namespace RefLib