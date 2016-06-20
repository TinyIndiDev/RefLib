#pragma once

#include <atomic>
#include <map>
#include <queue>
#include "reflib_safelock.h"

namespace RefLib
{

class NetConnection;

class NetConnectionMgr
{
public:
    NetConnectionMgr();
    ~NetConnectionMgr();

    bool Initialize(unsigned reserve);
    void Shutdown();

    NetConnection* GetNetConn();
    bool FreeNetConn(NetConnection *conn);

private:
    uint32 GetNextIndex();

    typedef std::queue<NetConnection*> FREE_CONNS;
    typedef std::map<uint64, NetConnection*> NET_CONNS;

    FREE_CONNS  _freeConns;
    NET_CONNS   _netConns;
    SafeLock    _connLock;

    std::atomic<uint32> _lastIndex;
    std::atomic<uint32> _capacity;
};

} // namespace RefLib