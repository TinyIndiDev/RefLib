#include "stdafx.h"

#include "reflib_net_socket_base.h"

namespace RefLib
{

/////////////////////////////////////////////////////////////////////
// NetSocketBase

void NetSocketBase::Disconnect(NetCloseType closer)
{
    g_network.Disconnect(GetSocket(), closer);
}

void NetSocketBase::OnDisconnected()
{
    SetSocket(INVALID_SOCKET);
    SetNetStatus(NET_STATUS_DISCONNECTED);
}

} // namespace RefLib