#include "stdafx.h"

#include "reflib_net_connection_proxy.h"
#include "reflib_net_connection.h"
#include "reflib_net_connection_manager.h"
#include "reflib_net_service.h"
#include "reflib_util.h"

namespace RefLib
{

NetConnectionProxy::NetConnectionProxy(NetService* container)
    : _container(container)
    , _isClosed(true)
{
    _conMgr = std::make_shared<NetConnectionMgr>();
}

NetConnectionProxy::~NetConnectionProxy()
{
}

bool NetConnectionProxy::Initialize(unsigned maxCnt)
{
    _isClosed = false;
    return _conMgr->Initialize(maxCnt);
}

std::weak_ptr<NetConnection> NetConnectionProxy::RegisterCon()
{
    return _conMgr->RegisterCon();
}

std::weak_ptr<NetConnection> NetConnectionProxy::AllocNetCon(SOCKET sock)
{
    auto con = _conMgr->AllocNetCon().lock();
    if (con && con->Initialize(sock, this))
        return con;

    return std::shared_ptr<NetConnection>();
}

void NetConnectionProxy::FreeNetCon(const CompositId& id)
{
    _conMgr->FreeNetCon(id);

    if (_isClosed && _conMgr->IsEmpty())
        OnTerminated();
}

void NetConnectionProxy::Shutdown()
{
    _isClosed = true;

    if (_conMgr->IsEmpty())
    {
        OnTerminated();
        return;
    }

    _conMgr->Shutdown();
}

void NetConnectionProxy::OnTerminated()
{
    if (_container)
        _container->OnTerminated(GetChildType());
}

} // namespace RefLib
