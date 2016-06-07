#include "stdafx.h"
#include "reflib_circular_buffer.h"
#include "reflib_util.h"
#include <algorithm>

namespace RefLib
{

CircularBuffer::CircularBuffer(int size)
    : _bufSize(size)
    , _headPos(0)
    , _tailPos(0)
{
    _buffer = new char[_bufSize];
}

CircularBuffer::~CircularBuffer()
{
    SAFE_DELETE_ARRAY(_buffer);
}

char CircularBuffer::GetStartData()
{
    return _buffer[_headPos];
}

char CircularBuffer::GetEndData()
{
    return (_tailPos > 0) ? _buffer[_tailPos - 1] : _buffer[_bufSize - 1];
}

char CircularBuffer::GetData(int offset)
{
    return _buffer[(_headPos + offset) % _bufSize];
}

void CircularBuffer::GetData(char *pData, int len)
{
    GetData(pData, len, 0);
}

void CircularBuffer::GetData(char *pData, int len, unsigned int offset)
{
    unsigned int localHeadPos = (_headPos + offset) % _bufSize;

    if (_tailPos < localHeadPos && localHeadPos + len > _bufSize)
    {
        int fc, sc;
        fc = _bufSize - localHeadPos;
        sc = len - fc;
        memcpy(pData, _buffer + localHeadPos, fc);
        if (sc > 0)
        {
            memcpy(pData + fc, _buffer, sc);
        }
    }
    else
    {
        memcpy(pData, _buffer + localHeadPos, len);
    }
}

char* CircularBuffer::GetDataSegment(unsigned int &len, unsigned int sizeLimit)
{
    if (_headPos == _tailPos)
    {
        len = 0;
        return 0;
    }
    else if (_headPos < _tailPos)
    {
        len = _tailPos - _headPos;
        if (len > sizeLimit) 
            len = sizeLimit;
        return _buffer + _headPos;
    }
    else
    {
        len = _bufSize - _headPos;
        if (len > sizeLimit) 
            len = sizeLimit;
        return _buffer + _headPos;
    }
}

void CircularBuffer::PutDataWithoutResize(const char *data, int len)
{
    if (_headPos == _tailPos)
    {
        memcpy(_buffer, data, len);
        _headPos = 0;
        _tailPos = len % _bufSize;
    }
    else if (_headPos < _tailPos && _tailPos + len >= _bufSize)
    {
        int copyLen1 = _bufSize - _tailPos;
        int copyLen2 = len - copyLen1;
        memcpy(_buffer + _tailPos, data, copyLen1);
        if (copyLen2 > 0)
        {
            memcpy(_buffer, data + copyLen1, copyLen2);
        }
        _tailPos = copyLen2;
    }
    else
    {
        memcpy(_buffer + _tailPos, data, len);
        _tailPos += len;
    }
}

bool CircularBuffer::PutData(const char *data, int len)
{
    if (len <= 0) return false;

    int room_size = static_cast<int>(_bufSize - Size());
    if (room_size <= len)
    {
        int extendSize = MAX_PACKET_SIZE * ((len - room_size) / MAX_PACKET_SIZE + 1);
        SetCapacity(_bufSize + extendSize);
    }
    PutDataWithoutResize(data, len);

    return true;
}

void CircularBuffer::SetCapacity(unsigned int size)
{
    if (size > _bufSize)
    {
        int prevBufSize = _bufSize;
        char *newData = new char[size];

        if (_headPos == _tailPos)
        {
            _tailPos = 0;
        }
        else if (_headPos < _tailPos)
        {
            memcpy(newData, _buffer + _headPos, _tailPos - _headPos);
            _tailPos -= _headPos;
        }
        else
        {
            memcpy(newData, _buffer + _headPos, prevBufSize - _headPos);
            memcpy(newData + (prevBufSize - _headPos), _buffer, _tailPos);
            _tailPos += (prevBufSize - _headPos);
        }
        _headPos = 0;

        delete[] _buffer;

        _buffer = newData;
        _bufSize = size;
    }
}

bool CircularBuffer::HeadIncrease(unsigned int increasement)
{
    if (increasement <= Size())
    {
        _headPos += increasement;
        _headPos %= _bufSize;
        return true;
    }
    else
    {
        return false;
    }
}

} // namespace RefLib
