#include "stdafx.h"
#include "reflib_circular_buffer.h"
#include <algorithm>

namespace RefLib
{

CircularBuffer::CircularBuffer(unsigned int size)
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

bool CircularBuffer::GetData(char *pData, unsigned int len)
{
    if (len > Size())
        return false;

    if (_tailPos < _headPos && _headPos + len > _bufSize)
    {
        int fc, sc;
        fc = _bufSize - _headPos;
        sc = len - fc;
        memcpy(pData, _buffer + _headPos, fc);
        if (sc > 0)
        {
            memcpy(pData + fc, _buffer, sc);
        }
    }
    else
    {
        memcpy(pData, _buffer + _headPos, len);
    }

    _headPos += len;

    return true;
}

bool CircularBuffer::PutData(const char *data, unsigned int len)
{
    REFLIB_ASSERT_RETURN_VAL_IF_FAILED(len > 0 || len <= MAX_PACKET_SIZE,
        "Cannot put data: Out of size", false);

    int room_size = _bufSize - Size();
    REFLIB_ASSERT_RETURN_VAL_IF_FAILED(room_size >= 0, 
        "Circular buffer corruption: room size is negative", false);

    if (static_cast<unsigned int>(room_size) <= len)
    {
        unsigned int extendSize = MAX_PACKET_SIZE * ((len - room_size) / MAX_PACKET_SIZE + 1);
        extendSize = (std::min)(extendSize, MAX_SOCKET_BUFFER_SIZE - _bufSize);
        REFLIB_ASSERT_RETURN_VAL_IF_FAILED(len >= room_size + extendSize, 
            "Cannot put data: reached extend limit of circular buffer", false);

        SetCapacity(_bufSize + extendSize);
    }

    PutDataWithoutResize(data, len);

    return true;
}

void CircularBuffer::PutDataWithoutResize(const char *data, unsigned int len)
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

        REFLIB_ASSERT_RETURN_IF_FAILED(copyLen1 > 0 && copyLen2 >= 0, "Circular buffer is corrupted.");

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

void CircularBuffer::SetCapacity(unsigned int size)
{
    if (size > _bufSize)
    {
        unsigned int prevBufSize = _bufSize;
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

} // namespace RefLib
