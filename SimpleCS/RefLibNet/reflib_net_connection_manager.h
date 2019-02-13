#pragma once

#include <atomic>
#include <vector>
#include <map>
#include <memory>
#include <concurrent_queue.h>
#include "reflib_safelock.h"
#include "reflib_composit_id.h"

namespace RefLib
{

class NetConnection;

class NetConnectionMgr
{
public:
    NetConnectionMgr();
    ~NetConnectionMgr();

    bool Initialize(uint32 capacity);
    std::weak_ptr<NetConnection> RegisterCon();
    void Shutdown();

    std::weak_ptr<NetConnection> AllocNetCon();
    std::weak_ptr<NetConnection> AllocNetCon(CompositId compId);
    void FreeNetCon(CompositId compId);

    bool IsEmpty();

private:
	typedef std::map<uint32, std::shared_ptr<NetConnection>> FREE_CONNS;
	typedef std::map<uint32, std::shared_ptr<NetConnection>> PENDING_CONNS;
	typedef std::map<uint64, std::shared_ptr<NetConnection>> BUSY_CONNS;

    FREE_CONNS		_freeCons;
	PENDING_CONNS	_pendingCons;
	BUSY_CONNS		_busyCons;
    SafeLock		_conLock;

	std::atomic<uint32> _capacity;
	std::atomic<bool>	_isActive;
};

} // namespace RefLib