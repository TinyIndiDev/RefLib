#pragma once

#include <atomic>

namespace RefLib
{

struct NetCompletionOP
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

	NetCompletionOP(NetOPType op_)
		: op(op_)
	{
		Reset();
	}

	void Reset(SOCKET sock = INVALID_SOCKET)
	{
		memset(&ol, 0x00, sizeof(WSAOVERLAPPED));
		client = sock;
	}

	NetCompletionOP& operator=(const NetCompletionOP& rhs)
	{
		if (this != &rhs)
		{
			memcpy(&ol, &rhs.ol, sizeof(WSAOVERLAPPED));
			client = rhs.client;
			op = rhs.op;
		}
		return *this;
	}

	WSAOVERLAPPED   ol;
	SOCKET          client;
	NetOPType       op;
};

class NetCompletionTarget
{
public:
    NetCompletionTarget() {}
    virtual ~NetCompletionTarget() {}

    virtual void OnCompletionSuccess(NetCompletionOP* bufObj, DWORD bytesTransfered) = 0;
    virtual void OnCompletionFailure(NetCompletionOP* bufObj, DWORD bytesTransfered, int error) = 0;
};

} // namespace RefLib

