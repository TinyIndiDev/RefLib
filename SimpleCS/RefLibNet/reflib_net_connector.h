#pragma once

#include <memory>
#include "reflib_net_completion.h"
#include "reflib_net_socket.h"

namespace RefLib
{

class NetAcceptor;
class NetCompletionOP;
class NetConnection;
class NetConnectionMgr;
class NetService;

class NetConnector
{
public:
    NetConnector(NetService* container);
    ~NetConnector();

    bool Initialize(unsigned maxCnt);
    bool Connect(const std::string& ipStr, uint32 port, std::weak_ptr<NetObj> obj);
    void Shutdown();

    void OnTerminated();

    std::weak_ptr<NetConnection> RegisterCon();

private:
    std::shared_ptr<NetConnectionMgr> _connMgr;
    NetService* _container;
};

} // namespace RefLib
