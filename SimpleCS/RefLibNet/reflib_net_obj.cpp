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
    if (auto p = _conn.lock())
        return p->GetCompId();

    return INVALID_OBJ_ID;
}

bool NetObj::Initialize(std::weak_ptr<NetConnection> conn)
{
    if (auto p = conn.lock())
    {
        _conn = p;
        return true;
    }
    return false;
}

// called afeter accept
bool NetObj::PostInit()
{
    if (auto p = _container.lock())
        return p->AllocNetObj(GetCompId());

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
    auto p = _conn.lock();
    REFLIB_ASSERT_RETURN_VAL_IF_FAILED(p, "NetConn is null", false);

    return p->Connect(sock, addr);
}

void NetObj::OnDisconnected()
{
    Reset();

    if (auto p = _container.lock())
        p->FreeNetObj(GetCompId());
}

// called by network thead
void NetObj::RecvPacket(MemoryBlock* packet)
{
    _recvPackets.push(packet);

    ::PostQueuedCompletionStatus(_comPort, 0, (ULONG_PTR)this, NULL);
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
    if (auto p = _conn.lock())
        p->Send(data, dataLen);
}

} //namespace RefLib
