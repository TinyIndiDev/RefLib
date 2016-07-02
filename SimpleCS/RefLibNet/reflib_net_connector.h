#pragma once

#include <memory>
#include "reflib_net_completion.h"
#include "reflib_net_socket.h"
#include "reflib_net_connection_proxy.h"

namespace RefLib
{

struct NetCompletionOP;
class NetAcceptor;

class NetConnector : public NetConnectionProxy
{
public:
    NetConnector(NetService* container);
    virtual ~NetConnector();

    virtual NetServiceChildType GetChildType() const { return NET_CTYPE_CONNECTOR; }

    bool Connect(const std::string& ipStr, uint32 port, std::weak_ptr<NetObj> obj);
};

} // namespace RefLib
