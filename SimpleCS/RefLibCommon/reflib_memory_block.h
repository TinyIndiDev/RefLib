#pragma once

namespace RefLib
{

class MemoryBlock
{
public:
    MemoryBlock();
    virtual ~MemoryBlock();

    void CreateMem(uint32 len);
    void DestroyMem();

    char* GetData() { return _data; }
    uint32 GetDataLen() const { return _dataLen; }

    void Resize(uint32 len);

private:
    char* _data;
    uint32 _dataLen;
    uint32 _capacity;
};

} // namespace RefLib
