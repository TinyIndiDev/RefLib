#include "stdafx.h"

#include "reflib_netio_buffer.h"
#include "reflib_memory_pool.h"
#include "reflib_packet_header_obj.h"

namespace RefLib
{

/////////////////////////////////////////////////////////////////////
// NetIoBuffer

NetIoBuffer::~NetIoBuffer()
{
	MemoryBlock* buffer;

	while (buffer = PopData())
	{
		g_memoryPool.FreeBuffer(buffer);
	}
}

MemoryBlock* NetIoBuffer::Alloc(int32 buffLen)
{
	MemoryBlock* buffer = g_memoryPool.GetBuffer(MAX_PACKET_SIZE);
	_data.push(buffer);

	return buffer;
}

MemoryBlock* NetIoBuffer::PopData()
{
	MemoryBlock* buffer = nullptr;

	if (!_data.empty())
	{
		buffer = _data.front();
		_data.pop();
	}
	return buffer;
}

} // namespace RefLib