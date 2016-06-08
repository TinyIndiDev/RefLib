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

    NetCompletionOP()
    {
        Reset();
    }

    void Reset()
    {
        NetOverlapped::Reset();
        _client = INVALID_SOCKET;
    }

    void SetOP(NetOPType op) { _op = op; }
    NetOPType GetOP() const { return _op; }

    void SetSocket(SOCKET sock) { _client = sock; }
    SOCKET GetSocket() const { return _client; }

private:
    NetOPType _op;
    SOCKET _client;
};

} // namespace RefLib
