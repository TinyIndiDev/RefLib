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

    virtual void OnCompletionSuccess(NetCompletionOP* bufObj, DWORD bytesTransfered) = 0;
    virtual void OnCompletionFailure(NetCompletionOP* bufObj, DWORD bytesTransfered, int error) = 0;
};

} // namespace RefLib

