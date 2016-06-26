#include "stdafx.h"

#include "reflib_net_worker_server.h"
#include "reflib_net_socket.h"
#include "reflib_net_service.h"
#include "reflib_net_api.h"

namespace RefLib
{

NetWorkerServer::NetWorkerServer(NetService* container)
    : _completionPort(INVALID_HANDLE_VALUE)
    , _container(container)
{
}

bool NetWorkerServer::Initialize(unsigned int concurrency)
{
    _completionPort = g_network.GetCompletionPort();
    if (_completionPort == INVALID_HANDLE_VALUE)
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

unsigned NetWorkerServer::Run()
{
    NetSocketBase* sockObj = nullptr;
    NetCompletionOP* bufObj = nullptr;
    OVERLAPPED *lpOverlapped = nullptr;
    
    DWORD   bytesTransfered;
    DWORD   flags;
    BOOL     rc;
    int     error;

    while (true)
    {
        error = NO_ERROR;

        rc = GetQueuedCompletionStatus(
            _completionPort,
            &bytesTransfered,
            (PULONG_PTR)&sockObj,
            &lpOverlapped,
            INFINITE);

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

bool NetWorkerServer::OnTerminated()
{
    if (_container)
        _container->OnTerminated(NET_CTYPE_NETWORKER);

    return true;
}

} // RefLib
