#pragma once

#include "reflib_net_def.h"

namespace RefLib
{

class CircularBuffer
{
public:
    CircularBuffer(unsigned int size = DEF_SOCKET_BUFFER_SIZE);
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

    bool GetData(char *pData, unsigned int len);
    bool PutData(const char *pData, unsigned int len);

private:
    void PutDataWithoutResize(const char *pData, unsigned int len);

    void SetCapacity(unsigned int size);
    unsigned int GetCapacity() const { return _bufSize; }

    unsigned int _bufSize;
    char *_buffer;

    unsigned int _headPos;
    unsigned int _tailPos;
};

} // namespace RefLib

