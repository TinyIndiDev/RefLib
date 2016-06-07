#include "stdafx.h"

#include "reflib_net_include.h"
#include "reflib_net_worker_server.h"
#include "reflib_net_socket.h"

namespace RefLib
{

NetWorkerServer::NetWorkerServer()
    : _completionPort(INVALID_HANDLE_VALUE)
{
}

bool NetWorkerServer::Initialize()
{
    SYSTEM_INFO sysInfo;

    _completionPort = g_network.GetCompletionPort();
    if (_completionPort == INVALID_HANDLE_VALUE)
    {
        DebugPrint("Completion port is null");
        return false;
    }

    GetSystemInfo(&sysInfo);
    if (sysInfo.dwNumberOfProcessors > NETWORK_MAX_COMPLETION_THREAD_COUNT)
        sysInfo.dwNumberOfProcessors = NETWORK_MAX_COMPLETION_THREAD_COUNT;

    if (!CreateThreads(sysInfo.dwNumberOfProcessors))
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
    int     rc;
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
            DebugPrint("CompletionThread: GetQueuedCompletionStatus failed: %d\n", GetLastError());
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
        DebugPrint("OP = %d; Error = %d\n", bufObj->GetOP(), error);

        sockObj->DecOps();
        return;
    }

    _bytesSent.fetch_add(bytesTransfered);
    _bytesSentLast.fetch_add(bytesTransfered);

    sockObj->OnCompletion(bufObj, bytesTransfered);
    sockObj->DecOps();
}

} // RefLib
