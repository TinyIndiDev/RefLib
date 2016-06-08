#pragma once

#include <map>
#include "reflib_singleton.h"

namespace RefLib
{

class AcceptBuffer;
class NetCompletionOP;

class NetworkAPI : public Singleton<NetworkAPI>
{
public:
    NetworkAPI();
    ~NetworkAPI();

    bool Initialize();

    HANDLE GetCompletionPort() const { return _completionPort; }

    bool Listen(SOCKET listenSock, const SOCKADDR_IN& saLocal);
    BOOL Accept(SOCKET listenSock, AcceptBuffer* acceptObj);
    void Disconnect(NetCompletionOP* bufObj, NetCloseType closer);

private:
    bool InitNetworkExFns();
    void DisconnectEx(NetCompletionOP* bufObj);

    LPFN_ACCEPTEX               _lpfnAcceptEx;
    LPFN_GETACCEPTEXSOCKADDRS   _lpfnGetAcceptExSockaddrs;
    LPFN_DISCONNECTEX           _lpfnDisconnectEx;

    HANDLE  _completionPort;
    WSADATA _wsd;

    bool _initialized;
};

} // namespace RefLib

#define g_network Singleton<NetworkAPI>::GetSingleton()