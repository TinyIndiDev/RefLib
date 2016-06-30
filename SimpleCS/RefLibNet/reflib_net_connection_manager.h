#pragma once

#include <atomic>
#include <map>
#include <deque>
#include <memory>
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

    bool Initialize(unsigned reserve);
    std::weak_ptr<NetConnection> RegisterCon();
    void Shutdown();

    std::weak_ptr<NetConnection> AllocNetCon();
    std::weak_ptr<NetConnection> AllocNetCon(const CompositId& compId);
    bool FreeNetCon(const CompositId& compId);

    bool IsEmpty();

private:
    uint32 GetNextIndex();

    typedef std::deque<std::shared_ptr<NetConnection>> FREE_CONNS;
    typedef std::map<uint64, std::shared_ptr<NetConnection>> NET_CONNS;

    FREE_CONNS  _freeCons;
    NET_CONNS   _netCons;
    SafeLock    _conLock;

    std::atomic<uint32> _lastIndex;
    std::atomic<uint32> _capacity;
};

} // namespace RefLib