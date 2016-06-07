#pragma once

#include <concurrent_queue.h>
#include "reflib_singleton.h"
#include "reflib_memory_block.h"

namespace RefLib
{

class MemoryPool : public Singleton<MemoryPool>
{
public:
    MemoryPool();
    ~MemoryPool();

    bool Initialize(unsigned int reserve);

    MemoryBlock* GetBuffer(unsigned int bufLen);
    void FreeBuffer(MemoryBlock* obj);

private:
    typedef Concurrency::concurrent_queue<MemoryBlock*> CONCURRENT_BUFFERS;

    CONCURRENT_BUFFERS _freeBuffers;
};

} // namespace RefLib

#define g_memoryPool Singleton<MemoryPool>::GetSingleton()