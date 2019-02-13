#pragma once

#include <concurrent_queue.h>
#include "reflib_net_socket_base.h"
#include "reflib_circular_buffer.h"
#include "reflib_safelock.h"

namespace RefLib
{

class NetObj;

class NetSocket : public NetSocketBase
{
public:
    NetSocket();
    virtual ~NetSocket() {}

    bool Initialize(SOCKET sock);

    void Send(char* data, uint16 dataLen);
    virtual bool RecvPacket(MemoryBlock* packet) { return true; }

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
    void OnSent(NetCompletionOP* sendOP, DWORD bytesTransfered);

    bool PostRecv();
    void OnRecv(NetCompletionOP* recvOP, DWORD bytesTransfered);

    void ClearRecvQueue();
    void ClearSendQueue();

    void OnRecvData(const char* data, int dataLen);
    ePACKET_EXTRACT_RESULT ExtractPakcetData(MemoryBlock*& buffer);

    Concurrency::concurrent_queue<MemoryBlock*> _sendQueue;
    Concurrency::concurrent_queue<MemoryBlock*> _sendPendingQueue;

    CircularBuffer  _recvBuffer;
    SafeLock        _recvLock;
};

} // namespace RefLib
