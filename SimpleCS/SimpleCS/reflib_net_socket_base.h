#pragma once

#include <atomic>
#include "reflib_net_completion.h"
#include "reflib_safelock.h"

namespace RefLib
{

class MemoryBlock;

class NetSocketBase : public NetCompletion
{
public:
    enum NetStatus
    {
        NET_STATUS_DISCONNECTED,
        NET_STATUS_CONNECTED,
        NET_STATUS_CLOSING,
    };

    NetSocketBase()
        : _socket(INVALID_SOCKET)
        , _netStatus(NET_STATUS_DISCONNECTED) {}
    virtual ~NetSocketBase() {}

    bool IsValid() const { return _socket != INVALID_SOCKET; }

    SOCKET GetSocket() const { return _socket; }
    void SetSocket(SOCKET sock) { _socket.exchange(sock); }

    int GetStatus() { return _netStatus; }
    void SetNetStatus(NetStatus status) { _netStatus.exchange(status); }

    void Disconnect(NetCloseType closer);
    virtual void OnDisconnected();

private:
    std::atomic<NetStatus> _netStatus;
    std::atomic<SOCKET> _socket;
};

} // namespace RefLib

