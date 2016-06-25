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

// called by NetSocket::OnRecvData()
void NetConnection::RecvPacket(MemoryBlock* packet) 
{
    auto p = _parent.lock();
    REFLIB_ASSERT_RETURN_IF_FAILED(p, "RevPaket: parent is nullptr");

    // deliver packet to GameNetObj
    p->RecvPacket(packet);
}

void NetConnection::OnDisconnected()
{
    NetSocket::OnDisconnected();

    auto p = _parent.lock();
    REFLIB_ASSERT(p, "OnDisconnected: parent is nullptr");
    if (p) p->OnDisconnected();

    auto container = _container.lock();
    REFLIB_ASSERT_RETURN_IF_FAILED(p, "OnDisconnected: container is nullptr");
    container->FreeNetConn(GetCompId());
}

} // namespace RefLib