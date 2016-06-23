#include "stdafx.h"

#include "reflib_net_connection.h"
#include "reflib_net_connection_manager.h"
#include "reflib_game_net_obj.h"

namespace RefLib
{

void NetConnection::RegisterParent(std::weak_ptr<GameNetObj> parent)
{
    _parent = parent;
}

bool NetConnection::Initialize(SOCKET sock, std::weak_ptr<NetConnectionMgr> container)
{
    if (!NetSocket::Initialize(sock))
        return false;

    if (!container.lock().get())
        return false;

    _container = container;

    if (auto p = _parent.lock())
        return p->PostInit();

    return false;
}

void NetConnection::RecvPacket(MemoryBlock* packet) 
{
    if (auto p = _parent.lock())
        p->RecvPacket(packet);
}

void NetConnection::OnDisconnected()
{
    NetSocket::OnDisconnected();

    if(auto container = _container.lock())
        container->PushNetConn(GetCompId());

    if (auto p = _parent.lock())
        p->OnDisconnected();
}

} // namespace RefLib