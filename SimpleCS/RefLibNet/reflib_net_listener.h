#pragma once

#include "reflib_net_completion.h"
#include "reflib_net_socket_base.h"

namespace RefLib
{

class NetAcceptor;
class NetConnection;
class NetConnectionMgr;
class NetCompletionOP;
class GameObjMgr;

class NetListener : public NetSocketBase
{
public:
    NetListener(GameObjMgr* gameObjMgr);
    ~NetListener();

    bool Initialize(unsigned reserve);
    bool Listen(unsigned port);
    void Shutdown();

    void FreeNetConn(NetConnection* conn);

    virtual void OnCompletionSuccess(NetCompletionOP* bufObj, DWORD bytesTransfered) override;
    virtual void OnCompletionFailure(NetCompletionOP* bufObj, DWORD bytesTransfered, int error) override;

private:
    void OnAccept(NetCompletionOP* bufObj);

    GameObjMgr*         _gameObjMgr;
    NetConnectionMgr*   _netConnMgr;
    NetAcceptor*        _acceptor;
};

} // namespace RefLib
