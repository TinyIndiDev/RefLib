#include "stdafx.h"

#include "reflib_net_worker.h"
#include "reflib_net_socket.h"
#include "reflib_net_service.h"
#include "reflib_net_api.h"
#include "reflib_def.h"

namespace RefLib
{

NetWorker::NetWorker(NetService* container)
    : _comPort(INVALID_HANDLE_VALUE)
    , _container(container)
{
}

bool NetWorker::Initialize(unsigned int concurrency)
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

void NetWorker::Run()
{
    NetSocketBase* sockObj = nullptr;
    OVERLAPPED* lpOverlapped = nullptr;
    DWORD bytesTransfered;
    int error = NO_ERROR;
    DWORD flags;

    BOOL rc = GetQueuedCompletionStatus(_comPort, &bytesTransfered,
        (PULONG_PTR)&sockObj, &lpOverlapped, THREAD_TIMEOUT_IN_MSEC);

    if (rc == FALSE)
    {
        // Check time out
        if (lpOverlapped == nullptr)
            return;

        rc = WSAGetOverlappedResult(sockObj->GetSocket(), lpOverlapped, 
            &bytesTransfered, FALSE, &flags);
        if (rc == FALSE)
        {
            error = WSAGetLastError();
        }
    }

    HandleIO(sockObj, lpOverlapped, bytesTransfered, error);
}

void NetWorker::HandleIO(NetSocketBase* sockObj, OVERLAPPED* lpOverlapped, DWORD bytesTransfered, int error)
{
    REFLIB_ASSERT_RETURN_IF_FAILED(sockObj, "NetSocket is null");

    NetCompletionOP* bufObj = CONTAINING_RECORD(lpOverlapped, NetCompletionOP, ol);

    if (error != NO_ERROR)
    {
        if (bytesTransfered == 0)
            sockObj->OnDisconnected();
        else           
            sockObj->OnCompletionFailure(bufObj, bytesTransfered, error);
    }
    else
    {
        _bytesSent.fetch_add(bytesTransfered);
        _bytesSentLast.fetch_add(bytesTransfered);

        sockObj->OnCompletionSuccess(bufObj, bytesTransfered);
    }
}

void NetWorker::OnDeactivated()
{
    if (_container)
        _container->OnTerminated(NET_CTYPE_NETWORKER);
}

} // RefLib
