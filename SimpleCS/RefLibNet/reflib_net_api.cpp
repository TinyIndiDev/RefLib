#include "stdafx.h"

#include "reflib_net_acceptor.h"
#include "reflib_net_api.h"

namespace RefLib
{

NetworkAPI::NetworkAPI()
    : _initialized(false)
    , _completionPort(INVALID_HANDLE_VALUE)
    , _lpfnAcceptEx(nullptr)
    , _lpfnGetAcceptExSockaddrs(nullptr)
    , _lpfnDisconnectEx(nullptr)
{
}

NetworkAPI::~NetworkAPI()
{
    if (_initialized)
    {
        if (WSACleanup() == SOCKET_ERROR)
        {
            DebugPrint("WSACleanup failed with error %s", SocketGetLastErrorString());
        }
    }
}

bool NetworkAPI::Initialize()
{
    if (WSAStartup(MAKEWORD(2, 2), &_wsd) != 0)
    {
        DebugPrint("unable to load Winsock!");
        return false;
    }

    _initialized = true;

    _completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, (ULONG_PTR)nullptr, 0);
    if (_completionPort == nullptr)
    {
        DebugPrint("CreateIoCompletionPort failed: %d", GetLastError());
        return false;
    }

    return InitNetworkExFns();
}

// Any SOCKET works
bool NetworkAPI::InitNetworkExFns()
{
    GUID guidAcceptEx = WSAID_ACCEPTEX;
    GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
    GUID guidDisconnectEx = WSAID_DISCONNECTEX;
    DWORD bytes;
    int rc;

    // Create the client socket for an incoming connection
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        DebugPrint("InitNetExFn failed: %s", SocketGetLastErrorString());
        return false;
    }

    // Need to load the Winsock extension functions from each provider
    rc = WSAIoctl(
        sock,
        SIO_GET_EXTENSION_FUNCTION_POINTER,
        &guidAcceptEx,
        sizeof(guidAcceptEx),
        &_lpfnAcceptEx,
        sizeof(_lpfnAcceptEx),
        &bytes,
        NULL,
        NULL
        );
    if (rc == SOCKET_ERROR)
    {
        DebugPrint("WSAIoctl: SIO_GET_EXTENSION_FUNCTION_POINTER failed: %s",
            SocketGetLastErrorString());
        return false;
    }

    rc = WSAIoctl(
        sock,
        SIO_GET_EXTENSION_FUNCTION_POINTER,
        &guidGetAcceptExSockaddrs,
        sizeof(guidGetAcceptExSockaddrs),
        &_lpfnGetAcceptExSockaddrs,
        sizeof(_lpfnGetAcceptExSockaddrs),
        &bytes,
        NULL,
        NULL
        );
    if (rc == SOCKET_ERROR)
    {
        DebugPrint("WSAIoctl: SIO_GET_EXTENSION_FUNCTION_POINTER faled: %s",
            SocketGetLastErrorString());
        return false;
    }

    WSAIoctl(
        sock,
        SIO_GET_EXTENSION_FUNCTION_POINTER,
        &guidDisconnectEx,
        sizeof(guidDisconnectEx),
        &_lpfnDisconnectEx,
        sizeof(_lpfnDisconnectEx),
        &bytes,
        NULL,
        NULL);
    if (rc == SOCKET_ERROR)
    {
        DebugPrint("WSAIoctl: SIO_GET_EXTENSION_FUNCTION_POINTER faled: %s",
            SocketGetLastErrorString());
        return false;
    }

    closesocket(sock);

    return true;
}

bool NetworkAPI::Listen(SOCKET listenSock, const SOCKADDR_IN& saLocal)
{
    if (listenSock == INVALID_SOCKET)
    {
        DebugPrint("Listen failed: Listen socket is null.");
        return false;
    }

    // bind the socket to a local address and port
    int rc = bind(listenSock, (SOCKADDR*)&saLocal, sizeof(saLocal));
    if (rc == SOCKET_ERROR)
    {
        DebugPrint("bind failed: %s", SocketGetLastErrorString());
        return false;
    }

    rc = listen(listenSock, NETWORK_DEF_BACKLOG);
    if (rc == SOCKET_ERROR)
    {
        DebugPrint("listen failed: %s", SocketGetLastErrorString());
        return false;
    }

    return true;
}

BOOL NetworkAPI::Accept(SOCKET listenSock, AcceptBuffer* acceptObj)
{
    DWORD bytes;

    if (listenSock == INVALID_SOCKET)
    {
        DebugPrint("Accept failed: Listen socket is null.");
        return false;
    }

    if (!acceptObj)
    {
        DebugPrint("Accept failed: accept buffer is null.");
        return false;
    }

    return _lpfnAcceptEx(
        listenSock,
        acceptObj->GetSocket(),
        acceptObj->GetData(),
        0,
        SOCKETADDR_BUFFER_SIZE,
        SOCKETADDR_BUFFER_SIZE,
        &bytes,
        reinterpret_cast<LPOVERLAPPED>(acceptObj)
        );
}

void NetworkAPI::Disconnect(NetCompletionOP* bufObj, NetCloseType closer)
{
    SOCKET socket = bufObj->GetSocket();

    if (socket == INVALID_SOCKET)
        return;

    if (closer == NET_CTYPE_SHUTDOWN)
    {
        shutdown(socket, SD_BOTH);
        closesocket(socket);
    }
    else
    {
        DisconnectEx(bufObj);
        closesocket(socket);
    }
}

void NetworkAPI::DisconnectEx(NetCompletionOP* bufObj)
{
    SOCKET socket = bufObj->GetSocket();

    // TODO: use overlapped to support asynchronus operation
    _lpfnDisconnectEx(socket, reinterpret_cast<LPOVERLAPPED>(bufObj), 0, 0);
}

} // namespace RefLib