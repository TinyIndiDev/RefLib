#include "stdafx.h"
#include "reflib_net_listener.h"
#include "reflib_net_acceptor.h"
#include "reflib_net_connection_manager.h"
#include "reflib_net_connection.h"
#include "reflib_game_obj.h"
#include "reflib_game_obj_manager.h"

namespace RefLib
{

NetListener::NetListener(GameObjMgr* gameObjMgr)
    : _acceptor(nullptr)
    , _netConnMgr(nullptr)
    , _gameObjMgr(gameObjMgr)
{
}

NetListener::~NetListener()
{
    SAFE_DELETE(_acceptor);
    SAFE_DELETE(_netConnMgr);
}

bool NetListener::Initialize(unsigned reserve)
{
    _netConnMgr = new NetConnectionMgr();
    if (!_netConnMgr->Initialize(reserve))
        return false;

    return true;
}

bool NetListener::Listen(unsigned port)
{
    SOCKADDR_IN saLocal;
    saLocal.sin_family = AF_INET;
    saLocal.sin_port = htons(port);
    saLocal.sin_addr.s_addr = htonl(INADDR_ANY);

    HANDLE completionPort = g_network.GetCompletionPort();
    if (completionPort == INVALID_HANDLE_VALUE)
    {
        DebugPrint("Completion port is null");
        return false;
    }

    SOCKET sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sClient == INVALID_SOCKET)
    {
        DebugPrint("Cannot create listen socket: %s", SocketGetLastErrorString());
        return false;
    }

    SetSocket(sClient);

    // Associate the socket and its NetSocket to the completion port
    HANDLE hrc = CreateIoCompletionPort((HANDLE)GetSocket(), completionPort, (ULONG_PTR)this, 0);
    if (hrc == NULL)
    {
        DebugPrint("CreateIoCompletionPort failed: %d\n", GetLastError());
        return false;
    }

    if (!g_network.Listen(GetSocket(), saLocal))
        return false;

    _acceptor = new NetAcceptor(reinterpret_cast<NetSocketBase*>(this), completionPort);
    _acceptor->Accepts();

    return true;
}

void NetListener::OnCompletionFailure(NetCompletionOP* bufObj, DWORD bytesTransfered, int error)
{
    DebugPrint("OP = %d; Error = %d\n", bufObj->GetOP(), error);
    return;
}

void NetListener::OnCompletionSuccess(NetCompletionOP* bufObj, DWORD bytesTransfered)
{
    switch (bufObj->GetOP())
    {
    case NetCompletionOP::OP_ACCEPT:
        OnAccept(bufObj);
        break;
    case NetCompletionOP::OP_CONNECT:
        OnConnected();
        break;
    case NetCompletionOP::OP_DISCONNECT:
        OnDisconnected();
        break;
    default:
        REFLIB_ASSERT(false, "Invalid net op");
        break;
    }
}

void NetListener::OnAccept(NetCompletionOP* bufObj)
{
    // Get a new SOCKET_OBJ for the client connection
    NetConnection* clientObj = _netConnMgr->GetNetConn();
    if (!clientObj)
    {
        DebugPrint("Out of net connection pool.");
        return;
    }

    if (!clientObj->Initialize(bufObj->GetSocket(), this))
    {
        DebugPrint("NetConnection initialization failed.");
        _netConnMgr->FreeNetConn(clientObj);
        return;
    }

    GameObj* obj = _gameObjMgr->GetGameObj();
    if (!obj)
    {
        DebugPrint("Out of game object pool.");
        _netConnMgr->FreeNetConn(clientObj);
        return;
    }

    if (!obj->Initialize(clientObj))
    {
        DebugPrint("Game object intiailization failed.");
        _gameObjMgr->FreeGameObj(obj);
        _netConnMgr->FreeNetConn(clientObj);
        return;
    }

    clientObj->SetParent(obj);
    _acceptor->OnAccept(clientObj, bufObj);
}

void NetListener::FreeNetConn(NetConnection* conn)
{
    REFLIB_ASSERT_RETURN_IF_FAILED(conn, "NetConnection is null");
    _netConnMgr->FreeNetConn(conn);
}

void NetListener::Shutdown()
{
    Disconnect(NET_CTYPE_SHUTDOWN);
    _netConnMgr->Shutdown();
}

} // namespace RefLib
