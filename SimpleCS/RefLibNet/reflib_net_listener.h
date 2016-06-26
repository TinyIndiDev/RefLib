#pragma once

#include <memory>
#include "reflib_net_completion.h"
#include "reflib_net_socket_base.h"

namespace RefLib
{

class NetAcceptor;
class NetCompletionOP;
class NetConnection;
class NetConnectionMgr;

class NetListener : public NetSocketBase
{
public:
    NetListener();
    ~NetListener();

    bool Initialize(unsigned maxCnt);
    bool Listen(unsigned port);
    void Shutdown();

    std::weak_ptr<NetConnection> RegisterCon();

    virtual void OnCompletionSuccess(NetCompletionOP* bufObj, DWORD bytesTransfered) override;
    virtual void OnCompletionFailure(NetCompletionOP* bufObj, DWORD bytesTransfered, int error) override;

private:
    void OnAccept(NetCompletionOP* bufObj);

    std::shared_ptr<NetConnectionMgr> _connMgr;
    std::unique_ptr<NetAcceptor> _acceptor;
};

} // namespace RefLib
