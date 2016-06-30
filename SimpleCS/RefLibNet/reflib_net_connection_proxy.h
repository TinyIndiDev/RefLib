#pragma once

#include <memory>
#include "reflib_composit_id.h"

namespace RefLib
{

class NetObj;
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
    bool AllocNetCon(const CompositId& id, SOCKET sock);
    void FreeNetCon(const CompositId& id);

    virtual NetServiceChildType GetChildType() const { return NET_CTYPE_NA; };
    virtual bool Listen(unsigned port) { return false; }
    virtual bool Connect(const std::string& ipStr, uint32 port, std::weak_ptr<NetObj> obj) { return false; }
    virtual void Shutdown();

    void OnTerminated();

private:
    std::shared_ptr<NetConnectionMgr> _conMgr;
    NetService* _container;
    bool _isClosed;
};

}
