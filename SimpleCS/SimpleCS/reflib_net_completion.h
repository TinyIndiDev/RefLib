#pragma once

#include <atomic>
#include "reflib_net_overlapped.h"

namespace RefLib
{

class NetCompletion
{
public:
    NetCompletion() {}
    virtual ~NetCompletion() {}

    void IncOps()
    {
        _outstandingOps.fetch_add(1);
    }

    void DecOps()
    {
        _outstandingOps.fetch_sub(1);
    }

    virtual void OnCompletion(NetCompletionOP* bufObj, DWORD bytesTransfered) = 0;

private:
    std::atomic<long> _outstandingOps;
};

} // namespace RefLib

