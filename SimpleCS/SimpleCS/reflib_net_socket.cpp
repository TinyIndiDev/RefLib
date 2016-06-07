#include "stdafx.h"

#include <list>
#include "reflib_net_socket.h"
#include "reflib_net_overlapped.h"
#include "reflib_net_listener.h"
#include "reflib_memory_pool.h"
#include "reflib_util.h"

namespace RefLib
{

/////////////////////////////////////////////////////////////////////
// NetSocket

NetIoBuffer::~NetIoBuffer()
{
    MemoryBlock* buffer = nullptr;

    while (PopData(buffer))
    {
        g_memoryPool.FreeBuffer(buffer);
    }
}

bool NetSocket::Initialize(SOCKET sock) 
{ 
    REFLIB_ASSERT_RETURN_VAL_IF_FAILED(sock != INVALID_SOCKET, "Socket is invalid", false);
    NetSocketBase::SetSocket(sock);

    return true;
}

void NetSocket::OnConnected()
{
    SetNetStatus(NET_STATUS_CONNECTED);
    PostRecv();
}

// TODO: This fn should be called when socket is actually closed.
void NetSocket::OnDisconnected()
{
    NetSocketBase::OnDisconnected();

    ClearRecvQueue();
    ClearSendQueue();
}

void NetSocket::ClearRecvQueue()
{
    SafeLock::Owner guard(_recvLock);
    _recvBuffer.Clear();
}

void NetSocket::ClearSendQueue()
{
    MemoryBlock* buffer = nullptr;

    REFLIB_ASSERT(_sendPendingQueue.empty(), "Send pending queue is not empty");
    while (_sendPendingQueue.try_pop(buffer))
    {
        SAFE_DELETE(buffer);
    }
}

void NetSocket::Send(char* data, uint32 dataLen)
{
    MemoryBlock* buffer = g_memoryPool.GetBuffer(dataLen);
    memcpy(buffer->GetData(), data, dataLen);

    _sendPendingQueue.push(buffer);

    // TODO: trigger PostSend
}

void NetSocket::OnCompletion(NetCompletionOP* bufObj, DWORD bytesTransfered)
{
    NetIoBuffer *ioBuffer = reinterpret_cast<NetIoBuffer*>(bufObj);

    switch (ioBuffer->GetOP())
    {
    case NetCompletionOP::OP_READ:
        OnRecv(ioBuffer, bytesTransfered);
        break;
    case NetCompletionOP::OP_WRITE:
        OnSent(ioBuffer, bytesTransfered);
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

    DecOps();
}

bool NetSocket::PostRecv()
{
    NetIoBuffer* recvOP = new NetIoBuffer();
    recvOP->SetSocket(GetSocket());
    recvOP->SetOP(NetCompletionOP::OP_READ);

    MemoryBlock* buffer = g_memoryPool.GetBuffer(MAX_PACKET_SIZE);
    recvOP->PushData(buffer);

    WSABUF wbuf;
    wbuf.buf = buffer->GetData();
    wbuf.len = buffer->GetDataLen();

    DWORD flags;
    int rc = WSARecv(
        GetSocket(),
        &wbuf,
        1,
        NULL,
        &flags,
        reinterpret_cast<WSAOVERLAPPED*>(recvOP),
        NULL
        );

    if (rc == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            DebugPrint("PostRecv: WSARecv* failed: %s", SocketGetLastErrorString());
            Disconnect(NET_CTYPE_SYSTEM);
            delete recvOP;

            return false;
        }
    }

    // Increment the outstanding operation count
    IncOps();

    return true;
}

void NetSocket::OnRecv(NetIoBuffer* recvOP, DWORD bytesTransfered)
{
    REFLIB_ASSERT_RETURN_IF_FAILED(recvOP, "NetIoBuffer is null");

    MemoryBlock* buffer = nullptr;
    if (recvOP->PopData(buffer))
    {
        SafeLock::Owner guard(_recvLock);
        _recvBuffer.PutData(buffer->GetData(), bytesTransfered);

        // Trigger reading data from business logic.
    }
    else
    {
        Disconnect(NetCloseType::NET_CTYPE_SYSTEM);
    }

    delete recvOP;

    PostRecv();
}

bool NetSocket::PostSend()
{
    std::list<MemoryBlock*> sendQueue;
    unsigned sendPacketSize = 0;
    unsigned idx = 0;
    MemoryBlock* buffer = nullptr;

    while (!_sendPendingQueue.empty() 
        && (idx < MAX_SEND_ARRAY_SIZE) 
        && (sendPacketSize < MAX_SOCKET_BUFFER_SIZE))
    {
        _sendPendingQueue.try_pop(buffer);
        sendQueue.push_back(buffer);

        sendPacketSize += buffer->GetDataLen();
        idx++;
    }

    size_t sendQueueSize = sendQueue.size();
    if (sendQueueSize == 0)
        return false;

    NetIoBuffer* sendOP = new NetIoBuffer();
    sendOP->SetSocket(GetSocket());
    sendOP->SetOP(NetCompletionOP::OP_WRITE);

    std::vector<WSABUF> wbufs;
    wbufs.reserve(sendQueueSize);
    idx = 0;
    for (auto element : sendQueue)
    {
        wbufs[idx].buf = buffer->GetData();
        wbufs[idx].len = buffer->GetDataLen();
        sendOP->PushData(buffer);
        idx++;
    }
    sendQueue.clear();

    int rc = WSASend(
        GetSocket(),
        &(wbufs[0]),
        static_cast<DWORD>(wbufs.size()),
        NULL,
        0,
        reinterpret_cast<LPOVERLAPPED>(sendOP),
        NULL
        );

    if (rc == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            DebugPrint("PostSend: WSASend* failed: %s", SocketGetLastErrorString());
            Disconnect(NET_CTYPE_SYSTEM);
            delete sendOP;

            return false;
        }
    }

    // Increment the outstanding operation count
    IncOps();

    return true;
}

void NetSocket::OnSent(NetIoBuffer* sendOP, DWORD bytesTransfered)
{
    delete sendOP;

    PostSend();
}

} // namespace RefLib