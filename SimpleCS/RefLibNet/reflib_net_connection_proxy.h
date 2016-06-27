#pragma once

#include <memory>
#include "reflib_composit_id.h"

namespace RefLib
{

class NetConnection;
class NetConnectionMgr;
class NetService;

class NetConnectionProxy
{
public:
    NetConnectionProxy(NetService* container);
    virtual ~NetConnectionProxy();

    bool Initialize(unsigned maxCnt);
    std::weak_ptr<NetConnection> RegisterCon();
    std::weak_ptr<NetConnection> AllocNetCon(SOCKET sock);
    void FreeNetCon(const CompositId& id);
    void OnTerminated();

    virtual NetServiceChildType GetChildType() const { return NET_CTYPE_NA; };
    virtual void Shutdown();

private:
    std::shared_ptr<NetConnectionMgr> _conMgr;
    NetService* _container;
    bool _isClosed;
};

}
