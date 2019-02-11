#include "stdafx.h"

#include "reflib_net_profiler.h"

#include <sysinfoapi.h>

namespace RefLib
{

NetProfiler::NetProfiler()
{
    ResetProfile();
}

void NetProfiler::ResetProfile()
{
    _bytesRead = 0;
    _bytesSent = 0;
    _startTime = 0;
    _bytesReadLast = 0;
    _bytesSentLast = 0;
    _startTimeLast = 0;
}

void NetProfiler::StartProfile()
{
    ResetProfile();
    _startTime = _startTimeLast = GetTickCount64();
}

void NetProfiler::PrintStatistics()
{
    uint64 tick = GetTickCount64();
    uint64 elapsed = (tick > _startTime) ? (tick - _startTime) / 1000 : 0;
    if (elapsed == 0)
        return;

    // Calculate average bytes per second
    uint64 bps = _bytesSent / elapsed;
    DebugPrint("Average BPS sent: %llu [%llu]", bps, _bytesSent.load());

    bps = _bytesRead / elapsed;
    DebugPrint("Average BPS read: %llu [%llu]", bps, _bytesRead.load());

    elapsed = (tick > _startTimeLast) ? (tick - _startTimeLast) / 1000 : 0;
    if (elapsed == 0)
        return;

    // Calculate bytes per second over the last X seconds
    bps = _bytesSentLast / elapsed;
    DebugPrint("Current BPS sent: %llu", bps);

    bps = _bytesReadLast / elapsed;
    DebugPrint("Current BPS read: %llu", bps);

    _bytesSentLast = 0;
    _bytesReadLast = 0;

    _startTimeLast = tick;
}

} // namespace RefLib