#include "stdafx.h"

#include <list>
#include "reflib_net_socket.h"
#include "reflib_net_overlapped.h"
#include "reflib_net_listener.h"
#include "reflib_memory_pool.h"
#include "reflib_packet_obj.h"
#include "reflib_game_obj.h"

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
    : _parent(nullptr)
{

}

bool NetSocket::Initialize(SOCKET sock) 
{ 
    REFLIB_ASSERT_RETURN_VAL_IF_FAILED(sock != INVALID_SOCKET, "Socket is invalid", false);
    NetSocketBase::SetSocket(sock);

    return true;
}

void NetSocket::SetParent(GameObj* parent)
{
    _parent = parent;
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
    REFLIB_ASSERT_RETURN_IF_FAILED(_parent, "No game object assigned.");

    SafeLock::Owner guard(_recvLock);

    _recvBuffer.PutData(data, dataLen);

    MemoryBlock* buffer = nullptr;
    bool error;

    while ((buffer = ExtractPakcetData(error)))
    {
        if (error)
        {
            Disconnect(NET_CTYPE_SYSTEM);
            break;
        }
        _parent->RecvPacket(buffer);
    }
}

bool NetSocket::CheckPacketData(char* blob, unsigned int len, uint16& contentLen, bool& error)
{
    PacketObj obj;

    if (!obj.ReadHeader(blob, len))
        return false;

    if (!obj.IsValidEnvTag())
    {
        DebugPrint("Received data is corrupted");
        error = true;
        return false;
    }

    contentLen = obj.GetContentLen();
    if (contentLen > len)
        return false;

    if (contentLen > MAX_PACKET_CONTENT_SIZE)
    {
        error = true;
        return false;
    }

    return true;
}

MemoryBlock* NetSocket::ExtractPakcetData(bool& error)
{
    error = false;

    unsigned int len = _recvBuffer.Size();
    if (len == 0)
        return nullptr;

    MemoryBlock* buffer = nullptr;
    char* blob = nullptr;
    bool isOverlapped = _recvBuffer.IsOverlapped();

    if (!isOverlapped)
    {
        unsigned int linearLen;
        _recvBuffer.GetLinearData(blob, linearLen, MAX_PACKET_SIZE);
        REFLIB_ASSERT_RETURN_VAL_IF_FAILED(!blob || len != linearLen, "Circulur buffer logic error", false);

        uint16 contentLen;
        if (!CheckPacketData(blob, len, contentLen, error))
            return nullptr;

        buffer = g_memoryPool.GetBuffer(contentLen);
        memcpy(buffer->GetData(), blob + PacketObj::GetHeaderSize(), contentLen);
    }
    else
    {
        buffer = g_memoryPool.GetBuffer(len);

        blob = buffer->GetData();
        _recvBuffer.GetData(blob, len);

        uint16 contentLen;
        if (!CheckPacketData(blob, len, contentLen, error))
        {
            g_memoryPool.FreeBuffer(buffer);
            return nullptr;
        }

        buffer->Resize(contentLen);
    }

    return buffer;
}

void NetSocket::Send(char* data, uint16 dataLen)
{
    PacketObj packet;
    packet.SetHeader(dataLen);

    MemoryBlock* buffer = g_memoryPool.GetBuffer(dataLen + packet.GetHeaderSize());
    memcpy(buffer->GetData(), &packet.header, packet.GetHeaderSize());
    memcpy(buffer->GetData() + packet.GetHeaderSize(), data, dataLen);

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