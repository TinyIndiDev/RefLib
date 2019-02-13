#pragma once

#include <queue>
#include "reflib_net_completion.h"

namespace RefLib
{

class MemoryBlock;

class NetIoBuffer : public NetCompletionOP
{
public:
	NetIoBuffer(NetOPType op) : NetCompletionOP(op) {}
	~NetIoBuffer();

	MemoryBlock* Alloc(int32 buffLen);

	void PushData(MemoryBlock* data) { _data.push(data); }
	inline MemoryBlock* PopData();

private:

	std::queue<MemoryBlock*> _data;
};

} // namespace RefLib
