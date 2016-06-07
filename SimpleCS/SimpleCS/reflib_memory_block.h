#pragma once

namespace RefLib
{

class MemoryBlock
{
public:
    MemoryBlock();
    virtual ~MemoryBlock();

    void CreateMem(uint32 len);
    bool DestroyMem();

    char* GetData() { return _data; }
    uint32 GetDataLen() const { return _dataLen; }

private:
    char* _data;
    uint32 _dataLen;
};

} // namespace RefLib
