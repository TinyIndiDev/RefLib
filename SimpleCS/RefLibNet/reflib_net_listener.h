#pragma once

#include <memory>
#include "reflib_net_completion.h"
#include "reflib_net_socket_base.h"
#include "reflib_net_connection_proxy.h"

namespace RefLib
{

struct NetCompletionOP;
class NetAcceptor;

class NetListener : public NetSocketBase, public NetConnectionProxy
{
public:
    NetListener(NetService* container);
    virtual ~NetListener();

    virtual NetServiceChildType GetChildType() const { return NET_CTYPE_LISTENER; }
    virtual void Shutdown() override;

    bool Listen(unsigned port);

    virtual void OnCompletionSuccess(NetCompletionOP* bufObj, DWORD bytesTransfered) override;
    virtual void OnCompletionFailure(NetCompletionOP* bufObj, DWORD bytesTransfered, int error) override;

private:
    void OnAccept(NetCompletionOP* bufObj);

    std::unique_ptr<NetAcceptor> _acceptor;
};

} // namespace RefLib
