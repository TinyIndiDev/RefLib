#include "stdafx.h"

#include "reflib_net_acceptor.h"
#include "reflib_net_connection.h"
#include "reflib_net_api.h"

namespace RefLib
{

NetAcceptor::NetAcceptor(NetSocketBase* sock, HANDLE complPort)
    : _listenSock(sock)
    , _completionPort(complPort)
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
    _pendingAccepts.reserve(NETWORK_DEFAULT_OVERLAPPED_COUNT);

    for (int i = 0; i < NETWORK_DEFAULT_OVERLAPPED_COUNT; ++i)
    {
        _pendingAccepts[i] = new AcceptBuffer;
        PostAccept(_pendingAccepts[i]);
    }
}

// Post an overlapped accept on a listening socket.
int NetAcceptor::PostAccept(AcceptBuffer* acceptObj)
{
    // Create the client socket for an incoming connection
    SOCKET sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sClient == INVALID_SOCKET)
    {
        DebugPrint("PostAccept failed: %s", SocketGetLastErrorString());
        return -1;
    }

    acceptObj->Reset();
    acceptObj->SetOP(NetCompletionOP::OP_ACCEPT);
    acceptObj->SetSocket(sClient);

    if (g_network.Accept(_listenSock->GetSocket(), acceptObj) == FALSE)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            DebugPrint("PostAccept: AcceptEx failed: %s", SocketGetLastErrorString());
            return SOCKET_ERROR;
        }
    }

    // Increment the outstanding operation count
    _listenSock->IncOps();

    return NO_ERROR;
}

void NetAcceptor::OnAccept(std::weak_ptr<NetConnection> clientObj, NetCompletionOP* bufObj)
{
    auto conn = clientObj.lock();
    if (!conn.get())
        return;

    // Associate the new connection to our completion port
    HANDLE hrc = CreateIoCompletionPort(
        (HANDLE)bufObj->GetSocket(),
        _completionPort,
        (ULONG_PTR)conn.get(),
        0);
    if (hrc == NULL)
    {
        DebugPrint("CompletionThread: CreateIoCompletionPort failed: %s", SocketGetLastErrorString());
        return;
    }

    conn->OnConnected();

    // Re-post the AcceptEx
    AcceptBuffer* acceptObj = reinterpret_cast<AcceptBuffer*>(bufObj);
    PostAccept(acceptObj);
}

} // namespace RefLib