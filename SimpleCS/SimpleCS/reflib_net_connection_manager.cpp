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
    SafeLock::Owner owner(_connLock);

    while (!_freeConns.empty())
    {
        NetConnection* conn = _freeConns.front();
        _freeConns.pop();
        SAFE_DELETE(conn);
    }

    for (auto element : _netConns)
    {
        delete element.second;
    }
    _netConns.clear();
}

bool NetConnectionMgr::Initialize(unsigned reserve)
{
    _capacity = reserve;

    SafeLock::Owner owner(_connLock);

    for (unsigned i = 0; i < reserve; ++i)
    {
        NetConnection* conn = new NetConnection(GetNextIndex(), 0);
        _freeConns.push(conn);
    }

    return true;
}

void NetConnectionMgr::Shutdown()
{
    SafeLock::Owner owner(_connLock);

    for (auto element : _netConns)
    {
        NetConnection* conn = element.second;
        conn->Disconnect(NET_CTYPE_SHUTDOWN);
    }
}

uint32 NetConnectionMgr::GetNextIndex()
{
    uint32 prev = _lastIndex.fetch_add(1);
    return (prev + 1);
}

NetConnection* NetConnectionMgr::GetNetConn()
{
    NetConnection *conn = nullptr;

    SafeLock::Owner owner(_connLock);

    if (!_freeConns.empty())
    {
        conn = _freeConns.front();
        _freeConns.pop();

        conn->IncSalt();
    }
    else if (_netConns.size() < _capacity)
    {
        conn = new NetConnection(GetNextIndex(), 0);
    }

    if (conn)
    {
        _netConns.insert(std::pair<uint64, NetConnection*>(conn->GetConId(), conn));
    }

    return conn;
}

bool NetConnectionMgr::FreeNetConn(NetConnection *conn)
{
    SafeLock::Owner owner(_connLock);

    auto it = _netConns.find(conn->GetConId());
    if (it != _netConns.end())
    {
        _netConns.erase(it);
        _freeConns.push(conn);

        return true;
    }

    return false;
}

} // namespace RefLib
