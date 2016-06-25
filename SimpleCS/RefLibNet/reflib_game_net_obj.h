#pragma once

#include <memory>
#include <concurrent_queue.h>
#include "reflib_composit_id.h"

namespace RefLib
{
class NetConnection;
class NetService;
class MemoryBlock;

class GameNetObj
{
public:
    GameNetObj(std::weak_ptr<NetService> container);
    virtual ~GameNetObj();

    CompositId GetCompId() const;
    std::weak_ptr<NetConnection> GetConn() { return _conn; }

    virtual bool Initialize(std::weak_ptr<NetConnection> conn);
    virtual bool PostInit();
    virtual void OnRecvPacket();
    virtual void Send(char* data, uint16 dataLen);
    virtual void OnDisconnected();

    void RecvPacket(MemoryBlock* packet);

protected:
    virtual void Reset();

    Concurrency::concurrent_queue<MemoryBlock*> _recvPackets;

    std::weak_ptr<NetConnection> _conn;
    HANDLE _comPort;

    std::weak_ptr<NetService> _container;
};

} //namespace RefLib
