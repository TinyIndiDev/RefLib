#include "stdafx.h"
#include "reflib_net_listener.h"
#include "reflib_net_acceptor.h"
#include "reflib_net_connection.h"
#include "reflib_net_connection_manager.h"
#include "reflib_net_obj.h"
#include "reflib_net_api.h"
#include "reflib_util.h"

namespace RefLib
{

NetListener::NetListener()
{
}

NetListener::~NetListener()
{
}

bool NetListener::Initialize(unsigned maxCnt)
{
    _connMgr = std::make_shared<NetConnectionMgr>();

    return _connMgr->Initialize(maxCnt);
}

std::weak_ptr<NetConnection> NetListener::RegisterNetConnection()
{
    return _connMgr ? _connMgr->Register() : std::weak_ptr<NetConnection>();
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

    _acceptor = std::make_unique<NetAcceptor>(reinterpret_cast<NetSocketBase*>(this), completionPort);
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
    if (!_connMgr) return;

    auto conn = _connMgr->AllocNetConn().lock();
    if (!conn) return;

    if (!conn->Initialize(bufObj->GetSocket(), _connMgr))
        return;

    _acceptor->OnAccept(conn, bufObj);
}

void NetListener::Shutdown()
{
    Disconnect(NET_CTYPE_SHUTDOWN);
    _connMgr->Shutdown();
}

} // namespace RefLib
