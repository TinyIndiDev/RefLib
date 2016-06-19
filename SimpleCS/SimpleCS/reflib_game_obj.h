#pragma once

#include <concurrent_queue.h>
#include "reflib_composit_id.h"

namespace RefLib
{
class NetConnection;
class MemoryBlock;

class GameObj
{
public:
    GameObj(const CompositId& id, HANDLE comPort);
    virtual ~GameObj();

    virtual bool Initialize(NetConnection* conn);

    virtual void Reset();

    virtual CompositId GetIndex() const { return _id; }

    virtual void RecvPacket(MemoryBlock* packet);
    virtual void Send(char* data, uint16 dataLen);

    virtual void OnRecvPacket();

protected:
    Concurrency::concurrent_queue<MemoryBlock*> _recvPackets;

    NetConnection* _conn;
    CompositId _id;

    HANDLE _comPort;
};

} //namespace RefLib
