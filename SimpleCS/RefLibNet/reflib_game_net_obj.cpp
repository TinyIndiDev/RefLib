#include "stdafx.h"
#include "reflib_game_net_obj.h"
#include "reflib_net_connection.h"
#include "reflib_net_connection_manager.h"
#include "reflib_net_service.h"
#include "reflib_def.h"

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
    {
        return p->GetCompId();
    }
    else
    {
        return INVALID_OBJ_ID;
    }
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
    // TODO: NetService::GetObj must be called
    return true;
}

// Called when game object is expired
void GameNetObj::Reset()
{
}

// called by network thead
void GameNetObj::RecvPacket(MemoryBlock* packet)
{
    _recvPackets.push(packet);

    ::PostQueuedCompletionStatus(_comPort, 0, (ULONG_PTR)this, NULL);
}

// called by business logic thread
void GameNetObj::OnRecvPacket()
{
}

void GameNetObj::Destroy()
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
