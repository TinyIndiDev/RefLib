#pragma once

#include <vector>
#include "reflib_net_overlapped.h"

#define SOCKETADDR_BUFFER_SIZE  (sizeof(SOCKADDR_STORAGE) + 16)

namespace RefLib
{

class NetSocketBase;
class NetConnection;

struct AcceptBuffer : public NetCompletionOP
{
    char data[SOCKETADDR_BUFFER_SIZE * 2];

    AcceptBuffer()
    {
        Reset();
    }

    void Reset()
    {
        NetCompletionOP::Reset();
        memset(data, 0x00, SOCKETADDR_BUFFER_SIZE * 2);
    }
};

class NetAcceptor
{
public:
    NetAcceptor(NetSocketBase* sock, HANDLE complPort);
    ~NetAcceptor();

    void Accepts();
    void OnAccept(NetConnection* clientobj, NetCompletionOP* bufObj);

private:
    int PostAccept(AcceptBuffer* acceptObj);

    std::vector<AcceptBuffer*> _pendingAccepts;
    NetSocketBase* _listenSock;
    HANDLE _completionPort;
};

} // namespace RefLib
