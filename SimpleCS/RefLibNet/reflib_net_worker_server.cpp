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

    if (!CreateThreads(concurrency))
        return false;

    NetProfiler::StartProfile();
    RunableThreads::Activate();

    return true;
}

void NetWorkerServer::Shutdown()
{
    RunableThreads::Deactivate();
    ::PostQueuedCompletionStatus(_comPort, 0, NULL, NULL);
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

    while (IsActive())
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
            continue;
        }

        bufObj = reinterpret_cast<NetCompletionOP*>(lpOverlapped);

        if (rc == FALSE)
        {
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

void NetWorkerServer::OnDeactivated()
{
    if (_container)
        _container->OnTerminated(NET_CTYPE_NETWORKER);
}

} // RefLib
