#pragma once

#include <concurrent_queue.h>
#include <queue>
#include "reflib_net_overlapped.h"
#include "reflib_net_socket_base.h"
#include "reflib_circular_buffer.h"
#include "reflib_safelock.h"

namespace RefLib
{

class GameNetObj;

class NetIoBuffer : public NetCompletionOP
{
public:
    NetIoBuffer(NetOPType op) : NetCompletionOP(op) {}
    ~NetIoBuffer();

    void PushData(MemoryBlock* data) 
    { 
        _data.push(data);
    }

    bool PopData(MemoryBlock* buffer) 
    {
        if (!_data.empty())
        {
            buffer = _data.front();
            _data.pop();
            return true;
        }
        return false;
    }

private:
    std::queue<MemoryBlock*> _data;
};

class NetSocket : public NetSocketBase
{
public:
    NetSocket();
    virtual ~NetSocket() {}

    bool Initialize(SOCKET sock);

    void Send(char* data, uint16 dataLen);
    virtual void RecvPacket(MemoryBlock* packet) {}

    virtual void OnCompletionSuccess(NetCompletionOP* bufObj, DWORD bytesTransfered) override;
    virtual void OnCompletionFailure(NetCompletionOP* bufObj, DWORD bytesTransfered, int error) override;

    virtual void OnConnected() override;
    virtual void OnDisconnected() override;

private:
    enum ePACKET_EXTRACT_RESULT
    {
        PER_SUCCESS,
        PER_NO_DATA,
        PER_ERROR,
    };
    void PrepareSend();
    bool PostSend();
    void OnSent(NetIoBuffer* sendOP, DWORD bytesTransfered);

    bool PostRecv();
    void OnRecv(NetIoBuffer* recvOP, DWORD bytesTransfered);

    void ClearRecvQueue();
    void ClearSendQueue();

    void OnRecvData(const char* data, int dataLen);
    ePACKET_EXTRACT_RESULT ExtractPakcetData(MemoryBlock* buffer);

    Concurrency::concurrent_queue<MemoryBlock*> _sendQueue;
    Concurrency::concurrent_queue<MemoryBlock*> _sendPendingQueue;

    CircularBuffer  _recvBuffer;
    SafeLock        _recvLock;
};

} // namespace RefLib
