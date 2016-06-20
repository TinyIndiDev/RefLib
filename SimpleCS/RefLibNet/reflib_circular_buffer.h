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

    bool IsOverlapped() const { return _tailPos < _headPos; }

    void GetData(char *pData, int len, unsigned int offset=0);
    void GetLinearData(char* data, unsigned int& len, unsigned int sizeLimit);

    bool PutData(const char *pData, int len);

private:
    void PutDataWithoutResize(const char *pData, int len);

    void SetCapacity(unsigned int size);
    unsigned int GetCapacity() const { return _bufSize; }

    unsigned int _bufSize;
    char *_buffer;

    unsigned int _headPos;
    unsigned int _tailPos;
};

} // namespace RefLib

