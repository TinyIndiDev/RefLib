#include "stdafx.h"

#include "reflib_net_socket_base.h"
#include "reflib_net_overlapped.h"

namespace RefLib
{

/////////////////////////////////////////////////////////////////////
// NetSocketBase

NetSocketBase::NetSocketBase()
    : _socket(INVALID_SOCKET)
    , _netStatus(NET_STATUS_DISCONNECTED) 
{
    _connectOP = new NetCompletionOP;
    _disconnectOP = new NetCompletionOP;
}

void NetSocketBase::Connect()
{
    // TODO: fill the code
}

void NetSocketBase::Disconnect(NetCloseType closer)
{
    _netStatus.fetch_or(NET_STATUS_CLOSING);
    g_network.Disconnect(_disconnectOP, closer);
}

void NetSocketBase::OnConnected()
{
    _netStatus.fetch_or(NET_STATUS_CONNECTED);
}

void NetSocketBase::OnDisconnected()
{
    SetSocket(INVALID_SOCKET);

    _netStatus.fetch_and(~NET_STATUS_CONNECTED);
}

} // namespace RefLib
