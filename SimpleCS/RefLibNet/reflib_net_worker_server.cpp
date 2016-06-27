#include "stdafx.h"

#include "reflib_net_worker_server.h"
#include "reflib_net_socket.h"
#include "reflib_net_service.h"
#include "reflib_net_api.h"

namespace RefLib
{

NetWorkerServer::NetWorkerServer(NetService* container)
    : _comPort(INVALID_HANDLE_VALUE)
    , _container(container)
    , _activated(false)
{
}

bool NetWorkerServer::Initialize(unsigned int concurrency)
{
    _comPort = g_network.GetCompletionPort();
    if (_comPort == INVALID_HANDLE_VALUE)
    {
        DebugPrint("Completion port is null");
        return false;
    }

    _activated = true;
    if (!CreateThreads(concurrency))
        return false;

    NetProfiler::StartProfile();
    RunableThreads::Activate();

    return true;
}

unsigned NetWorkerServer::Run()
{
    NetSocketBase* sockObj = nullptr;
    NetCompletionOP* bufObj = nullptr;
    OVERLAPPED *lpOverlapped = nullptr;
    
    DWORD   bytesTransfered;
    DWORD   flags;
    BOOL     rc;
    int     error;

    while (_activated)
    {
        error = NO_ERROR;

        rc = GetQueuedCompletionStatus(
            _comPort,
            &bytesTransfered,
            (PULONG_PTR)&sockObj,
            &lpOverlapped,
            INFINITE);

        if (!sockObj)
        {
            DebugPrint("NetWorkerServer::Run received shutdown signal");
            break;
        }

        bufObj = reinterpret_cast<NetCompletionOP*>(lpOverlapped);

        if (rc == FALSE)
        {
            // If the call fails, call WSAGetOverlappedResult to translate the
            // error code into a Winsock error code.
            DebugPrint("CompletionThread: GetQueuedCompletionStatus failed: %d", GetLastError());
            rc = WSAGetOverlappedResult(
                sockObj->GetSocket(),
                lpOverlapped,
                &bytesTransfered,
                FALSE,
                &flags);
            if (rc == FALSE)
            {
                error = WSAGetLastError();
            }
        }

        HandleIO(sockObj, bufObj, bytesTransfered, error);
    }

    return 0;
}

void NetWorkerServer::HandleIO(NetSocketBase* sockObj, NetCompletionOP* bufObj, DWORD bytesTransfered, int error)
{
    REFLIB_ASSERT_RETURN_IF_FAILED(sockObj, "NetSocket is null");

    if (error != NO_ERROR)
    {
        sockObj->OnCompletionFailure(bufObj, bytesTransfered, error);
    }
    else
    {
        _bytesSent.fetch_add(bytesTransfered);
        _bytesSentLast.fetch_add(bytesTransfered);

        sockObj->OnCompletionSuccess(bufObj, bytesTransfered);
    }
}

bool NetWorkerServer::OnTimeout()
{
    if (!RunableThreads::OnTimeout())
        return false;

    _activated = false;

    if (_container)
        _container->OnTerminated(NET_CTYPE_NETWORKER);

    return true;
}

bool NetWorkerServer::OnTerminated()
{
    _activated = false;

    if (_container)
        _container->OnTerminated(NET_CTYPE_NETWORKER);

    return true;
}

} // RefLib
