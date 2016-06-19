#include "stdafx.h"

#include "reflib_net_connection.h"
#include "reflib_net_listener.h"
#include "reflib_game_obj.h"

namespace RefLib
{

bool NetConnection::Initialize(SOCKET sock, NetListener* netListener)
{
    if (!NetSocket::Initialize(sock))
        return false;

    REFLIB_ASSERT_RETURN_VAL_IF_FAILED(netListener, "NetListener is null", false);
    _container = netListener;
    return true;
}

void NetConnection::OnDisconnected()
{
    NetSocket::OnDisconnected();

    _container->FreeNetConn(this);
}

} // namespace RefLib