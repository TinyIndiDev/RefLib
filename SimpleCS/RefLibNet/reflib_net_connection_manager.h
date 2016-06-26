#pragma once

#include <atomic>
#include <map>
#include <queue>
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

    std::weak_ptr<NetConnection> AllocNetConn();
    bool FreeNetConn(CompositId compId);

private:
    uint32 GetNextIndex();

    typedef std::queue<std::shared_ptr<NetConnection>> FREE_CONNS;
    typedef std::map<uint64, std::shared_ptr<NetConnection>> NET_CONNS;

    FREE_CONNS  _freeConns;
    NET_CONNS   _netConns;
    SafeLock    _connLock;

    std::atomic<uint32> _lastIndex;
    std::atomic<uint32> _capacity;
};

} // namespace RefLib