#include "stdafx.h"
#include "reflib_net_connection_manager.h"
#include "reflib_net_connection.h"

namespace RefLib
{

NetConnectionMgr::NetConnectionMgr()
    : _capacity(NETWORK_MAX_CONN)
    , _lastIndex(0)
{
}

NetConnectionMgr::~NetConnectionMgr()
{
}

bool NetConnectionMgr::Initialize(unsigned reserve)
{
    _capacity = reserve;

    return true;
}

std::weak_ptr<NetConnection> NetConnectionMgr::Register()
{
    if (_capacity <= _lastIndex)
        return std::weak_ptr<NetConnection>();

    SafeLock::Owner owner(_connLock);

    auto p = std::make_shared<NetConnection>(GetNextIndex(), 0);
    _freeConns.push(p);

    return p;
}

void NetConnectionMgr::Shutdown()
{
    SafeLock::Owner owner(_connLock);

    for (auto element : _netConns)
    {
        auto conn = element.second;
        if (conn.get())
            conn->Disconnect(NET_CTYPE_SHUTDOWN);
    }
}

uint32 NetConnectionMgr::GetNextIndex()
{
    uint32 prev = _lastIndex.fetch_add(1);
    return (prev + 1);
}

std::weak_ptr<NetConnection> NetConnectionMgr::AllocNetConn()
{
    std::shared_ptr<NetConnection> conn;

    SafeLock::Owner owner(_connLock);

    if (!_freeConns.empty())
    {
        conn = _freeConns.front();
        _freeConns.pop();

        if (conn.get())
        {
            conn->IncSalt();
            _netConns.insert(std::pair<uint64, std::shared_ptr<NetConnection>>(conn->GetCompId().GetIndex(), conn));
        }
    }

    return conn;
}

bool NetConnectionMgr::FreeNetConn(CompositId compId)
{
    SafeLock::Owner owner(_connLock);

    auto it = _netConns.find(compId.GetIndex());
    if (it != _netConns.end())
    {
        _freeConns.push(it->second);
        _netConns.erase(it);

        return true;
    }

    return false;
}

} // namespace RefLib
