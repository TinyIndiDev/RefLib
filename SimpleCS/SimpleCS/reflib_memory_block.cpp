#include "stdafx.h"
#include "reflib_memory_block.h"

namespace RefLib
{

MemoryBlock::MemoryBlock()
    : _data(nullptr)
    , _dataLen(0)
{
}

MemoryBlock::~MemoryBlock()
{
    REFLIB_ASSERT(!_data, "Memory leak detected!");

    DestroyMem();
}

void MemoryBlock::CreateMem(uint32 len)
{
    DestroyMem();

    _data = new char[len];
    _dataLen = len;
}

bool MemoryBlock::DestroyMem()
{
    if (_data)
    {
        delete [] _data;
        _dataLen = 0;

        return true;
    }

    return false;
}

} // namespace RefLib
