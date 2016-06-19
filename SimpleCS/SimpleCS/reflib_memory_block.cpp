#include "stdafx.h"
#include "reflib_memory_block.h"

namespace RefLib
{

MemoryBlock::MemoryBlock()
    : _data(nullptr)
    , _dataLen(0)
    , _capacity(0)
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

    _capacity = len;
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

void MemoryBlock::Resize(uint32 len)
{
    if (len < _capacity)
    {
        _dataLen = len;
    }
    else if (len > _capacity)
    {
        char* data = new char[len];

        memcpy_s(data, len, _data, _dataLen);

        std::swap(_data, data);
        _capacity = len;
        _dataLen = len;

        delete[] data;
    }
}

} // namespace RefLib
