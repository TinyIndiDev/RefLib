#include "stdafx.h"
#include "reflib_net_obj.h"
#include "reflib_net_connection.h"
#include "reflib_net_connection_manager.h"
#include "reflib_net_service.h"
#include "reflib_def.h"
#include "reflib_memory_pool.h"

namespace RefLib
{

NetObj::NetObj(std::weak_ptr<NetService> container)
    : _comPort(INVALID_HANDLE_VALUE)
{
    if (auto p = container.lock())
    {
        _comPort = p->GetCompletionPort();
        _container = container;
    }
}

NetObj::~NetObj()
{
}

CompositId NetObj::GetCompId() const
{ 
    if (auto p = _con.lock())
        return p->GetCompId();

    return INVALID_OBJ_ID;
}

bool NetObj::Initialize(std::weak_ptr<NetConnection> con)
{
    if (auto p = con.lock())
    {
        _con = p;
        return true;
    }
    return false;
}

// Called when game object is expired
void NetObj::Reset()
{
    MemoryBlock* buffer = nullptr;

    REFLIB_ASSERT(_recvPackets.empty(), "Reset NetObj: recv packet queue is not empty");
    while (_recvPackets.try_pop(buffer))
    {
        g_memoryPool.FreeBuffer(buffer);
    }
}

bool NetObj::Connect(SOCKET sock, const SOCKADDR_IN& addr)
{
    auto p = _con.lock();
    REFLIB_ASSERT_RETURN_VAL_IF_FAILED(p, "NetConn is null", false);

    return p->Connect(sock, addr);
}

void NetObj::OnConnected()
{
    if (auto p = _container.lock())
        p->AllocNetObj(GetCompId());
}

void NetObj::OnDisconnected()
{
    Reset();

    if (auto p = _container.lock())
        p->FreeNetObj(GetCompId());
}

// called by network thead
bool NetObj::RecvPacket(MemoryBlock* packet)
{
    _recvPackets.push(packet);

    ::PostQueuedCompletionStatus(_comPort, 0, (ULONG_PTR)this, NULL);

    return true;
}

MemoryBlock* NetObj::PopRecvPacket()
{
    MemoryBlock* buffer = nullptr;
    if (_recvPackets.try_pop(buffer))
        return buffer;

    return nullptr;
}

void NetObj::Send(char* data, uint16 dataLen)
{
    if (auto p = _con.lock())
        p->Send(data, dataLen);
}

} //namespace RefLib
