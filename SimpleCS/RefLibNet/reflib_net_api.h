#pragma once

#include <map>
#include "loki_singleton.h"

namespace RefLib
{

class AcceptBuffer;
class NetCompletionOP;

class NetworkAPI
{
public:
    NetworkAPI();
    ~NetworkAPI();

    bool Initialize();

    HANDLE GetCompletionPort() const { return _comPort; }

    bool Listen(SOCKET listenSock, const SOCKADDR_IN& saLocal);
    bool Accept(SOCKET listenSock, AcceptBuffer* acceptObj);
    bool Connect(NetCompletionOP* bufObj, const SOCKADDR_IN& addr);
    bool Disconnect(NetCompletionOP* bufObj, NetCloseType closer);

private:
    bool InitNetworkExFns();
    void ConnectEx(NetCompletionOP* bufObj, const SOCKADDR_IN& addr);
    void DisconnectEx(NetCompletionOP* bufObj);

    LPFN_ACCEPTEX               _lpfnAcceptEx;
    LPFN_GETACCEPTEXSOCKADDRS   _lpfnGetAcceptExSockaddrs;
    LPFN_CONNECTEX              _lpfnConnectEx;
    LPFN_DISCONNECTEX           _lpfnDisconnectEx;

    HANDLE  _comPort;
    WSADATA _wsd;

    bool _initialized;
};

} // namespace RefLib

typedef Loki::SingletonHolder<RefLib::NetworkAPI> NetworkAPITypeSingleton;
#define g_network NetworkAPITypeSingleton::Instance()
