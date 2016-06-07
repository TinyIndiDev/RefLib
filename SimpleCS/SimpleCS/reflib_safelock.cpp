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

void SafeLock::Enter()
{
    ::EnterCriticalSection(&_crit);
}

void SafeLock::Leave()
{
    ::LeaveCriticalSection(&_crit);
}

} // namespace RefLib