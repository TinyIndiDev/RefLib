#include "stdafx.h"
#include "reflib_game_net_obj.h"
#include "reflib_net_connection.h"
#include "reflib_net_connection_manager.h"
#include "reflib_net_service.h"
#include "reflib_def.h"
#include "reflib_memory_pool.h"

namespace RefLib
{

GameNetObj::GameNetObj(std::weak_ptr<NetService> container)
    : _comPort(INVALID_HANDLE_VALUE)
{
    if (auto p = container.lock())
    {
        _comPort = p->GetCompletionPort();
        _container = container;
    }
}

GameNetObj::~GameNetObj()
{
}

CompositId GameNetObj::GetCompId() const 
{ 
    if (auto p = _conn.lock())
        return p->GetCompId();

    return INVALID_OBJ_ID;
}

bool GameNetObj::Initialize(std::weak_ptr<NetConnection> conn)
{
    if (auto p = conn.lock())
    {
        _conn = p;
        return true;
    }
    return false;
}

// called afeter accept
bool GameNetObj::PostInit()
{
    if (auto p = _container.lock())
        return p->AllocObj(GetCompId());

    return false;
}

// Called when game object is expired
void GameNetObj::Reset()
{
    MemoryBlock* buffer = nullptr;

    REFLIB_ASSERT(_recvPackets.empty(), "Reset GameNetObj: recv packet queue is not empty");
    while (_recvPackets.try_pop(buffer))
    {
        g_memoryPool.FreeBuffer(buffer);
    }
}

// called by network thead
void GameNetObj::RecvPacket(MemoryBlock* packet)
{
    _recvPackets.push(packet);

    ::PostQueuedCompletionStatus(_comPort, 0, (ULONG_PTR)this, NULL);
}

void GameNetObj::OnRecvPacket()
{
}

void GameNetObj::OnDisconnected()
{
    Reset();

    if (auto p = _container.lock())
        p->FreeObj(GetCompId());
}

void GameNetObj::Send(char* data, uint16 dataLen)
{
    if (auto p = _conn.lock())
        p->Send(data, dataLen);
}

} //namespace RefLib
