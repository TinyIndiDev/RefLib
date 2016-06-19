#pragma once

#include "reflib_type_def.h"
#include <limits>

namespace RefLib
{

#define PACKET_ENVELOP_TAG 0xffaa

class PacketObj
{
public:
    class Header
    {
    public:
        union TagType
        {
            uint16 envTag;
            char byte[2];
        };
        union LenType
        {
            uint16 contentLen;
            char byte[2];
        };

    public:
        TagType tag;
        LenType len;
    };

    void SetHeader(uint16 len)
    {
        header.tag.envTag = PACKET_ENVELOP_TAG;
        header.len.contentLen = len;
    }

    bool ReadHeader(char* data, uint16 len)
    {
        if (len < GetHeaderSize())
            return false;

        memcpy_s(&header, GetHeaderSize(), data, len);
        return true;
    }

    static uint16 GetHeaderSize()
    {
        return sizeof(header.tag.envTag) + sizeof(header.len.contentLen);
    }

    bool IsValidEnvTag() const
    {
        return (header.tag.envTag == PACKET_ENVELOP_TAG);
    }

    uint16 GetContentLen() const
    {
        return header.len.contentLen;
    }

public:
    Header header;
    char*  conetnt;
};

#define MAX_PACKET_CONTENT_SIZE ((1024)*(64) - PacketObj::GetHeaderSize())

}
