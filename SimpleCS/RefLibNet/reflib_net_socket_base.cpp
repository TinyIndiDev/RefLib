#include "stdafx.h"

#include "reflib_net_socket_base.h"
#include "reflib_net_overlapped.h"
#include "reflib_net_api.h"

namespace RefLib
{

/////////////////////////////////////////////////////////////////////
// NetSocketBase

NetSocketBase::NetSocketBase()
    : _socket(INVALID_SOCKET)
    , _netStatus(NET_STATUS_DISCONNECTED) 
{
    _connectOP = new NetCompletionOP(NetCompletionOP::OP_CONNECT);
    _disconnectOP = new NetCompletionOP(NetCompletionOP::OP_DISCONNECT);
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
