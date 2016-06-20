#pragma once

#include <atomic>
#include "reflib_net_completion.h"
#include "reflib_safelock.h"

namespace RefLib
{

class MemoryBlock;
class NetCompletionOP;

class NetSocketBase : public NetCompletion
{
public:
    NetSocketBase();
    virtual ~NetSocketBase() {}

    SOCKET GetSocket() const { return _socket; }
    void SetSocket(SOCKET sock) { _socket.exchange(sock); }

    void Connect();
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

