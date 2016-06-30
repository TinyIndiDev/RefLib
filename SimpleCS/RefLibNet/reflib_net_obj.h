#pragma once

#include <memory>
#include <concurrent_queue.h>
#include "reflib_composit_id.h"

namespace RefLib
{
class NetConnection;
class NetService;
class MemoryBlock;

class NetObj
{
public:
    NetObj(std::weak_ptr<NetService> container);
    virtual ~NetObj();

    CompositId GetCompId() const;
    std::weak_ptr<NetConnection> GetConn() { return _con; }

    virtual bool Initialize(std::weak_ptr<NetConnection> con);
    virtual bool PostInit();
    virtual bool Connect(SOCKET sock, const SOCKADDR_IN& addr);
    virtual bool OnRecvPacket()=0;
    virtual void Send(char* data, uint16 dataLen);
    virtual void OnConnected() {}
    virtual void OnDisconnected();

    bool RecvPacket(MemoryBlock* packet);
    MemoryBlock* PopRecvPacket();

private:
    void Reset();

    Concurrency::concurrent_queue<MemoryBlock*> _recvPackets;

    HANDLE _comPort;
    std::weak_ptr<NetConnection> _con;
    std::weak_ptr<NetService> _container;
};

} //namespace RefLib
