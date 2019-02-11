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
	for (size_t i = 0; i < reserve; ++i)
	{
		auto nextIndex = GetNextIndex();
		_freeCons.emplace(nextIndex, std::make_shared<NetConnection>(nextIndex, 0));
	}
	_capacity = reserve;

    return true;
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

std::weak_ptr<NetConnection> NetConnectionMgr::RegisterCon()
{
	SafeLock::Owner owner(_conLock);

	if (_freeCons.size() == 0)
	{
		return std::weak_ptr<NetConnection>();
	}

	return _freeCons.begin()->second;
}

std::weak_ptr<NetConnection> NetConnectionMgr::AllocNetCon()
{
    std::shared_ptr<NetConnection> con;

    SafeLock::Owner owner(_conLock);

    REFLIB_ASSERT_RETURN_VAL_IF_FAILED(!_freeCons.empty(), "Out of network connection.", con);

	auto it = _freeCons.begin();
	con = it->second;
	_freeCons.erase(it);

    con->IncSalt();
    _netCons.insert(std::pair<uint32, std::shared_ptr<NetConnection>>(con->GetCompId().GetSlotId(), con));

    return con;
}

std::weak_ptr<NetConnection> NetConnectionMgr::AllocNetCon(const CompositId& compId)
{
    std::shared_ptr<NetConnection> con;

    SafeLock::Owner owner(_conLock);

	auto it = _freeCons.find(compId.GetSlotId());
    if (it == _freeCons.end())
        return con;

	con = it->second;
	_freeCons.erase(it);

    con->IncSalt();
    _netCons.insert(std::pair<uint32, std::shared_ptr<NetConnection>>(con->GetCompId().GetSlotId(), con));

    return con;
}

bool NetConnectionMgr::FreeNetCon(const CompositId& compId)
{
    SafeLock::Owner owner(_conLock);

    auto it = _netCons.find(compId.GetSlotId());
    if (it != _netCons.end())
    {
		_freeCons[compId.GetSlotId()] = it->second;
		_netCons.erase(it);

        return true;
    }

    return false;
}

} // namespace RefLib
