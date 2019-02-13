#pragma once

#include <atomic>
#include "reflib_net_completion.h"
#include "reflib_safelock.h"

namespace RefLib
{

class MemoryBlock;

class NetSocketBase : public NetCompletionTarget
{
public:
    NetSocketBase();
    virtual ~NetSocketBase() {}

    void SetSocket(SOCKET sock);
    SOCKET GetSocket() const { return _socket; }

    bool Connect(SOCKET sock, const SOCKADDR_IN& addr);
    void Disconnect(NetCloseType closer);

    virtual void OnConnected();
    virtual void OnDisconnected();

protected:
    std::atomic<int> _netStatus;

private:
    NetCompletionOP* _connectOP;
    NetCompletionOP* _disconnectOP;

    std::atomic<SOCKET> _socket;
};

} // namespace RefLib

