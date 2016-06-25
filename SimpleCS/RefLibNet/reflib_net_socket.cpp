#include "stdafx.h"

#include <list>
#include "reflib_net_socket.h"
#include "reflib_net_overlapped.h"
#include "reflib_net_listener.h"
#include "reflib_memory_pool.h"
#include "reflib_packet_obj.h"

namespace RefLib
{

/////////////////////////////////////////////////////////////////////
// NetIoBuffer

NetIoBuffer::~NetIoBuffer()
{
    MemoryBlock* buffer = nullptr;

    while (PopData(buffer))
    {
        g_memoryPool.FreeBuffer(buffer);
    }
}

/////////////////////////////////////////////////////////////////////
// NetSocket

NetSocket::NetSocket()
{
}

bool NetSocket::Initialize(SOCKET sock) 
{ 
    REFLIB_ASSERT_RETURN_VAL_IF_FAILED(sock != INVALID_SOCKET, "Socket is invalid", false);
    NetSocketBase::SetSocket(sock);

    return true;
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

    REFLIB_ASSERT(_sendQueue.empty(), "Send queue is not empty");
    while (_sendQueue.try_pop(buffer))
    {
        SAFE_DELETE(buffer);
    }
}

bool NetSocket::PostRecv()
{
    _netStatus.fetch_or(NET_STATUS_RECV_PENDING);

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

void NetSocket::OnRecvData(const char* data, int dataLen)
{
    REFLIB_ASSERT_RETURN_IF_FAILED(data, "null data received.");
    REFLIB_ASSERT_RETURN_IF_FAILED(dataLen, "null size data received.");

    {
        SafeLock::Owner guard(_recvLock);
        _recvBuffer.PutData(data, dataLen);
    }

    MemoryBlock* buffer = nullptr;
    ePACKET_EXTRACT_RESULT ret = PER_NO_DATA;

    while ((ret = ExtractPakcetData(buffer)) == PER_SUCCESS)
    {
        RecvPacket(buffer);
    };

    if (ret == PER_ERROR)
        Disconnect(NET_CTYPE_SYSTEM);
}

NetSocket::ePACKET_EXTRACT_RESULT NetSocket::ExtractPakcetData(MemoryBlock* buffer)
{
    PacketObj packetObj;

    SafeLock::Owner guard(_recvLock);

    if (!_recvBuffer.GetData(packetObj.header.blob, PACKET_HEADER_SIZE))
    {
        buffer = nullptr;
        return PER_NO_DATA;
    }

    if (!packetObj.IsValidEnvTag())
    {
        DebugPrint("Invalid packet envelop tag");
        buffer = nullptr;
        return PER_ERROR;
    }

    if (packetObj.IsValidContentLength())
    {
        DebugPrint("Invalid packet conetnt length");
        buffer = nullptr;
        return PER_ERROR;
    }

    uint16 contentLen = packetObj.GetContentLen();

    buffer = g_memoryPool.GetBuffer(contentLen);
    _recvBuffer.GetData(buffer->GetData(), contentLen);

    return PER_SUCCESS;
}

void NetSocket::Send(char* data, uint16 dataLen)
{
    PacketObj packet;
    packet.SetHeader(dataLen);

    MemoryBlock* buffer = g_memoryPool.GetBuffer(dataLen + PACKET_HEADER_SIZE);

    memcpy(buffer->GetData(), packet.header.blob, PACKET_HEADER_SIZE);
    memcpy(buffer->GetData() + PACKET_HEADER_SIZE, data, dataLen);

    _sendPendingQueue.push(buffer);

    PrepareSend();
}

void NetSocket::PrepareSend()
{
    unsigned sendPacketSize = 0;
    MemoryBlock* buffer = nullptr;

    while (!_sendPendingQueue.empty()
        && (_sendQueue.unsafe_size() < MAX_SEND_ARRAY_SIZE)
        && (sendPacketSize < MAX_SOCKET_BUFFER_SIZE))
    {
        _sendPendingQueue.try_pop(buffer);
        _sendQueue.push(buffer);

        sendPacketSize += buffer->GetDataLen();
    }

    if (sendPacketSize > 0)
        PostSend();
}

bool NetSocket::PostSend()
{
    int status = _netStatus.load();
    int expected = status | (NET_STATUS_CONNECTED) & (~NET_STATUS_SEND_PENDING) & (~NET_STATUS_CLOSING);
    int desired = expected | NET_STATUS_SEND_PENDING;

    if (!_netStatus.compare_exchange_weak(expected, desired))
        return false;

    size_t sendQueueSize = _sendQueue.unsafe_size();
    if (sendQueueSize == 0)
        return false;

    NetIoBuffer* sendOP = new NetIoBuffer();
    sendOP->SetSocket(GetSocket());
    sendOP->SetOP(NetCompletionOP::OP_WRITE);

    std::vector<WSABUF> wbufs;
    wbufs.reserve(sendQueueSize);

    MemoryBlock* buffer = nullptr;
    int idx = 0;
    while (_sendQueue.try_pop(buffer))
    {
        wbufs[idx].buf = buffer->GetData();
        wbufs[idx].len = buffer->GetDataLen();
        sendOP->PushData(buffer);
        idx++;
    }

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

void NetSocket::OnCompletionFailure(NetCompletionOP* bufObj, DWORD bytesTransfered, int error)
{
    DebugPrint("OP = %d; Error = %d\n", bufObj->GetOP(), error);

    switch (bufObj->GetOP())
    {
    case NetCompletionOP::OP_READ:
    case NetCompletionOP::OP_WRITE:
        delete bufObj;
        break;
    default:
        REFLIB_ASSERT(false, "Invalid net op");
        break;
    }
    return;
}

void NetSocket::OnCompletionSuccess(NetCompletionOP* bufObj, DWORD bytesTransfered)
{
    NetIoBuffer *ioBuffer = reinterpret_cast<NetIoBuffer*>(bufObj);

    switch (ioBuffer->GetOP())
    {
    case NetCompletionOP::OP_CONNECT:
        OnConnected();
        break;
    case NetCompletionOP::OP_READ:
        OnRecv(ioBuffer, bytesTransfered);
        break;
    case NetCompletionOP::OP_WRITE:
        OnSent(ioBuffer, bytesTransfered);
        break;
    case NetCompletionOP::OP_DISCONNECT:
        OnDisconnected();
        break;
    default:
        REFLIB_ASSERT(false, "Invalid net op");
        break;
    }
}

void NetSocket::OnConnected()
{
    NetSocketBase::OnConnected();
    PostRecv();
}

void NetSocket::OnRecv(NetIoBuffer* recvOP, DWORD bytesTransfered)
{
    REFLIB_ASSERT_RETURN_IF_FAILED(recvOP, "NetIoBuffer is null");

    MemoryBlock* buffer = nullptr;
    if (recvOP->PopData(buffer))
    {
        OnRecvData(buffer->GetData(), bytesTransfered);
    }
    else
    {
        Disconnect(NetCloseType::NET_CTYPE_SYSTEM);
    }

    delete recvOP;
    _netStatus.fetch_and(~NET_STATUS_RECV_PENDING);

    PostRecv();
}

void NetSocket::OnSent(NetIoBuffer* sendOP, DWORD bytesTransfered)
{
    delete sendOP;
    _netStatus.fetch_and(~NET_STATUS_SEND_PENDING);

    PrepareSend();
}

void NetSocket::OnDisconnected()
{
    NetSocketBase::OnDisconnected();
    ClearRecvQueue();
    ClearSendQueue();
}

} // namespace RefLib