#include "stdafx.h"

#include "reflib_net_acceptor.h"
#include "reflib_net_connection.h"
#include "reflib_net_api.h"

namespace RefLib
{

NetAcceptor::NetAcceptor(NetSocketBase* sock, HANDLE compPort)
    : _listenSock(sock)
    , _comPort(compPort)
{
}

NetAcceptor::~NetAcceptor()
{
    for (auto element : _pendingAccepts)
    {
        delete element;
    }
    _pendingAccepts.clear();
}

void NetAcceptor::Accepts()
{
    _pendingAccepts.resize(NETWORK_DEFAULT_OVERLAPPED_COUNT, nullptr);

    for (int i = 0; i < NETWORK_DEFAULT_OVERLAPPED_COUNT; ++i)
    {
        _pendingAccepts[i] = new AcceptBuffer;
        PostAccept(_pendingAccepts[i]);
    }
}

// Post an overlapped accept on a listening socket.
bool NetAcceptor::PostAccept(AcceptBuffer* acceptObj)
{
    // Create the client socket for an incoming connection
    SOCKET sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sClient == INVALID_SOCKET)
    {
        DebugPrint("PostAccept failed: %s", SocketGetLastErrorString().c_str());
        return false;
    }

    acceptObj->Reset(sClient);

    if (g_network.Accept(_listenSock->GetSocket(), acceptObj) == false)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            DebugPrint("AcceptEx failed: %s", SocketGetLastErrorString().c_str());
            return false;
        }

        DebugPrint("PostAccept: socket(%d)", sClient);
    }

    return true;
}

void NetAcceptor::OnAccept(std::weak_ptr<NetConnection> clientObj, NetCompletionOP* bufObj)
{
    auto con = clientObj.lock();
    if (!con.get())
        return;

    // Associate the new connection to our completion port
    HANDLE hrc = CreateIoCompletionPort(
        (HANDLE)bufObj->client,
        _comPort,
        (ULONG_PTR)con.get(),
        0);
    if (hrc == NULL)
    {
        DebugPrint("OnAccept failed: %s", SocketGetLastErrorString().c_str());
        return;
    }

    con->OnConnected();

    // Re-post the AcceptEx
    AcceptBuffer* acceptObj = reinterpret_cast<AcceptBuffer*>(bufObj);
    PostAccept(acceptObj);
}

} // namespace RefLib