#include "stdafx.h"

#include "reflib_net_connection.h"
#include "reflib_net_connection_proxy.h"
#include "reflib_net_obj.h"

namespace RefLib
{

void NetConnection::RegisterParent(std::weak_ptr<NetObj> parent)
{
    _parent = parent;
}

bool NetConnection::Initialize(SOCKET sock, NetConnectionProxy* container)
{
    REFLIB_ASSERT_RETURN_VAL_IF_FAILED(container, "NetConnection::Initialize: NetConnectionProxy is null", false);
    _container = container;

    if (!NetSocket::Initialize(sock))
        return false;

    if (auto p = _parent.lock())
        return p->PostInit();

    return false;
}

// called by NetSocket::OnRecvData()
void NetConnection::RecvPacket(MemoryBlock* packet) 
{
    auto p = _parent.lock();
    REFLIB_ASSERT_RETURN_IF_FAILED(p, "RevPaket: parent is nullptr");

    // deliver packet to NetObj
    p->RecvPacket(packet);
}

void NetConnection::OnDisconnected()
{
    NetSocket::OnDisconnected();

    auto p = _parent.lock();
    REFLIB_ASSERT(p, "OnDisconnected: parent is nullptr");
    if (p) p->OnDisconnected();

    REFLIB_ASSERT_RETURN_IF_FAILED(_container, "OnDisconnected: container is nullptr");
    _container->FreeNetCon(GetCompId());
}

} // namespace RefLib