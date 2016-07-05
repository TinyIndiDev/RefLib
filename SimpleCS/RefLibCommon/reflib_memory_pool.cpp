#include "stdafx.h"

#include "reflib_memory_pool.h"

namespace RefLib
{

MemoryPool::MemoryPool()
{
}

MemoryPool::~MemoryPool()
{
    MemoryBlock* buffer = nullptr;

    while (_freeBuffers.try_pop(buffer))
    {
        SAFE_DELETE(buffer);
    }
}

bool MemoryPool::Initialize(unsigned int reserve)
{
    for (unsigned int i = 0; i < reserve; ++i)
    {
        MemoryBlock* buffer = new MemoryBlock();
        _freeBuffers.push(buffer);
    }

    return true;
}

MemoryBlock* MemoryPool::GetBuffer(unsigned int bufLen)
{
    MemoryBlock *newObj = nullptr;

    if (!_freeBuffers.try_pop(newObj))
    {
        newObj = new MemoryBlock();
    }
    newObj->CreateMem(bufLen);

    return newObj;
}

void MemoryPool::FreeBuffer(MemoryBlock *obj)
{
    REFLIB_ASSERT_RETURN_IF_FAILED(obj, "Netbuffer is null");

    obj->DestroyMem();
    _freeBuffers.push(obj);
}

} // namespace RefLib
