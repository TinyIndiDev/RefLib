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

void NetSocketBase::SetSocket(SOCKET sock)
{
    _netStatus = NET_STATUS_DISCONNECTED;
    _socket.exchange(sock); 
}

bool NetSocketBase::Connect(SOCKET sock, const SOCKADDR_IN& addr)
{
    SetSocket(sock);

    _netStatus.fetch_or(NET_STATUS_CONN_PENDING);
    _connectOP->Reset(_socket);
    return g_network.Connect(_connectOP, addr);
}

void NetSocketBase::Disconnect(NetCloseType closer)
{
    _netStatus.fetch_or(NET_STATUS_CLOSE_PENDING);
    _disconnectOP->Reset(_socket);
    g_network.Disconnect(_disconnectOP, closer);
}

void NetSocketBase::OnConnected()
{
	_netStatus.fetch_and(~NET_STATUS_CONN_PENDING);
	_netStatus.fetch_or(NET_STATUS_CONNECTED);
	DebugPrint("OnConnected: %d", _socket.load());
}

void NetSocketBase::OnDisconnected()
{
	if (_netStatus.load() == NET_STATUS_CONN_PENDING)
	{
		DebugPrint("Cannot connect");
	}
	else
	{
		_netStatus.fetch_and((~NET_STATUS_CONNECTED) & (~NET_STATUS_RECV_PENDING) & (~NET_STATUS_CLOSE_PENDING));
		if (_netStatus != 0)
		{
			DebugPrint("OnDisconnected: closed without clearing pending status 0x%x", _netStatus.load());
		}
		else
		{
			DebugPrint("OnDisconnected: %d", _socket.load());
		}
	}

	_socket.exchange(INVALID_SOCKET);
}

} // namespace RefLib)
