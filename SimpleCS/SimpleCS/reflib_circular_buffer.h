#pragma once

#include "reflib_net_def.h"

namespace RefLib
{

class CircularBuffer
{
public:
    CircularBuffer(int size = MAX_SOCKET_BUFFER_SIZE);
    virtual ~CircularBuffer();

    void Clear()
    {
        _headPos = 0;
        _tailPos = 0;
    }

    unsigned int Size() const
    {
        return (_headPos <= _tailPos) ? (_tailPos - _headPos) : (_bufSize - (_headPos - _tailPos));
    }

    unsigned int GetHeadPos() const { return _headPos; }
    unsigned int GetTailPos() const { return _tailPos; }

    char GetStartData();
    char GetEndData();

    char GetData(int offset);
    void GetData(char *pData, int len);
    void GetData(char *pData, int len, unsigned int offset);

    char* GetDataSegment(unsigned int &len, unsigned int sizeLimit);

    void PutDataWithoutResize(const char *pData, int len);
    bool PutData(const char *pData, int len);

    bool HeadIncrease(unsigned int increasement);

private:
    void SetCapacity(unsigned int size);
    unsigned int GetCapacity() const { return _bufSize; }

    unsigned int _bufSize;
    char *_buffer;

    unsigned int _headPos;
    unsigned int _tailPos;
};

} // namespace RefLib

