#pragma once

#include "reflib_type_def.h"
#include <atomic>

namespace RefLib
{

class NetProfiler
{
public:
    NetProfiler();

protected:
    void ResetProfile();
    void StartProfile();
    void PrintStatistics();

    std::atomic<uint64> _bytesRead;
    std::atomic<uint64> _bytesSent;
    std::atomic<uint64> _startTime;
    std::atomic<uint64> _bytesReadLast;
    std::atomic<uint64> _bytesSentLast;
    std::atomic<uint64> _startTimeLast;
};

} // namespace RefLib

