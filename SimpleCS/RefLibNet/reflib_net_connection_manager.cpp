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

std::weak_ptr<NetConnection> NetConnectionMgr::RegisterCon()
{
    if (_capacity <= _lastIndex)
        return std::weak_ptr<NetConnection>();

    SafeLock::Owner owner(_conLock);

    auto p = std::make_shared<NetConnection>(GetNextIndex(), 0);
    _freeCons.push_back(p);

    return p;
}

void NetConnectionMgr::Shutdown()
{
    SafeLock::Owner owner(_conLock);

    for (auto element : _netCons)
    {
        auto con = element.second;
        if (con.get())
            con->Disconnect(NET_CTYPE_SHUTDOWN);
    }
}

uint32 NetConnectionMgr::GetNextIndex()
{
    uint32 prev = _lastIndex.fetch_add(1);
    return (prev + 1);
}

bool NetConnectionMgr::IsEmpty()
{
    SafeLock::Owner owner(_conLock);
    
    return _netCons.empty();
}

std::weak_ptr<NetConnection> NetConnectionMgr::AllocNetCon()
{
    std::shared_ptr<NetConnection> con;

    SafeLock::Owner owner(_conLock);

    REFLIB_ASSERT_RETURN_VAL_IF_FAILED(!_freeCons.empty(), "Out of network connection.", con);

    con = _freeCons.front();
    _freeCons.pop_front();

    if (con.get())
    {
        con->IncSalt();
        _netCons.insert(std::pair<uint64, std::shared_ptr<NetConnection>>(con->GetCompId().GetIndex(), con));
    }

    return con;
}

std::weak_ptr<NetConnection> NetConnectionMgr::AllocNetCon(const CompositId& compId)
{
    std::shared_ptr<NetConnection> con;

    SafeLock::Owner owner(_conLock);

    auto it = _freeCons.begin();
    for (; it != _freeCons.end(); ++it)
    {
        auto p = it->get();
        if (p && (p->GetCompId() == compId))
            break;
    }
    if (it == _freeCons.end())
        return con;

    con = _freeCons.front();
    _freeCons.pop_front();

    if (con.get())
    {
        con->IncSalt();
        _netCons.insert(std::pair<uint64, std::shared_ptr<NetConnection>>(con->GetCompId().GetIndex(), con));
    }

    return con;
}

bool NetConnectionMgr::FreeNetCon(const CompositId& compId)
{
    SafeLock::Owner owner(_conLock);

    auto it = _netCons.find(compId.GetIndex());
    if (it != _netCons.end())
    {
        _freeCons.push_back(it->second);
        _netCons.erase(it);

        return true;
    }

    return false;
}

} // namespace RefLib
