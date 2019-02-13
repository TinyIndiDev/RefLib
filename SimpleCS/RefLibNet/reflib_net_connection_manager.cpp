#include "stdafx.h"
#include "reflib_net_connection_manager.h"
#include "reflib_net_connection.h"
#include "reflib_composit_id.h"

namespace RefLib
{

NetConnectionMgr::NetConnectionMgr()
	: _capacity(NETWORK_MAX_CONN)
	, _isActive(false)
{
}

NetConnectionMgr::~NetConnectionMgr()
{
}

bool NetConnectionMgr::Initialize(uint32 capacity)
{
	SafeLock::Owner owner(_conLock);

	for (uint32 i = 0; i < capacity; ++i)
	{
		_freeCons.emplace(i, std::make_shared<NetConnection>(i, 0));
	}

	_capacity = capacity;
	_isActive = true;

    return true;
}

void NetConnectionMgr::Shutdown()
{
	_isActive = false;

    SafeLock::Owner owner(_conLock);

	for (auto& elem : _busyCons)
	{
		auto con = elem.second;
        if (con.get())
            con->Disconnect(NET_CTYPE_SHUTDOWN);
    }
}

bool NetConnectionMgr::IsEmpty()
{
    SafeLock::Owner owner(_conLock);
    
	return (_freeCons.size() + _pendingCons.size() == _capacity);
}

std::weak_ptr<NetConnection> NetConnectionMgr::RegisterCon()
{
	SafeLock::Owner owner(_conLock);

	if (!_isActive || _freeCons.empty())
	{
		return std::weak_ptr<NetConnection>();
	}

	auto it = _freeCons.begin();
	auto con = it->second;
	_freeCons.erase(it);
	_pendingCons.emplace(con->GetCompId().GetSlotId(), con);

	return con;
}

std::weak_ptr<NetConnection> NetConnectionMgr::AllocNetCon()
{
    std::shared_ptr<NetConnection> con;

	if (!_isActive) 
	{
		return con;
	}

    SafeLock::Owner owner(_conLock);

    REFLIB_ASSERT_RETURN_VAL_IF_FAILED(!_pendingCons.empty(), "Out of network connection.", con);

	auto it = _pendingCons.begin();
	con = it->second;
	_pendingCons.erase(it);

    con->IncSalt();
	_busyCons.emplace(con->GetCompId().GetIndex(), con);

    return con;
}

std::weak_ptr<NetConnection> NetConnectionMgr::AllocNetCon(CompositId compId)
{
	std::shared_ptr<NetConnection> con;

	SafeLock::Owner owner(_conLock);

	if (!_isActive)
	{
		return con;
	}

	auto it = _pendingCons.find(compId.GetSlotId());
	if (it == _pendingCons.end())
	{
		return con;
	}

	con = it->second;
	_pendingCons.erase(it);

	con->IncSalt();
	_busyCons.emplace(con->GetCompId().GetIndex(), con);

	return con;
}

void NetConnectionMgr::FreeNetCon(CompositId compId)
{
	SafeLock::Owner owner(_conLock);

	auto it = _busyCons.find(compId.GetIndex());
	if (it != _busyCons.end())
	{
		auto con = it->second;
		_busyCons.erase(it);
		_freeCons.emplace(compId.GetSlotId(), con);
	}
}

} // namespace RefLib
