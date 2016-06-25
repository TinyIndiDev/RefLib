#pragma once

namespace RefLib
{

class NetOverlapped : public WSAOVERLAPPED
{
public:
    NetOverlapped()
    {
        Reset();
    }

    void Reset()
    {
        memset(this, 0x00, sizeof(NetOverlapped));
    }
};

class NetCompletionOP : public NetOverlapped
{
public:
    enum NetOPType
    {
        OP_ACCEPT = 0,
        OP_CONNECT,
        OP_READ,
        OP_WRITE,
        OP_DISCONNECT,
    };

    NetCompletionOP(NetOPType op)
        : _op(op)
    {
        Reset();
    }

    void Reset()
    {
        NetOverlapped::Reset();
        _client = INVALID_SOCKET;
    }

    NetOPType GetOP() const { return _op; }

    void SetSocket(SOCKET sock) { _client = sock; }
    SOCKET GetSocket() const { return _client; }

private:
    const NetOPType _op;
    SOCKET _client;
};

} // namespace RefLib
