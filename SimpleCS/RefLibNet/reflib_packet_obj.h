#pragma once

#include "reflib_type_def.h"
#include <limits>

namespace RefLib
{

#define PACKET_ENVELOP_TAG      0xffaa
#define PACKET_HEADER_SIZE      PacketObj::GetHeaderSize()
#define MAX_PACKET_CONTENT_SIZE ((1024)*(64) - PACKET_HEADER_SIZE)

class PacketObj
{
public:
    static uint16 GetHeaderSize()
    {
        return sizeof(header.blob);
    }

    void SetHeader(uint16 contentLen)
    {
        header.info.envTag = PACKET_ENVELOP_TAG;
        header.info.contentLen = contentLen;
    }

    bool IsValidEnvTag() const
    {
        return (header.info.envTag == PACKET_ENVELOP_TAG);
    }

    bool IsValidContentLength() const
    {
        if (header.info.contentLen + PACKET_HEADER_SIZE > header.info.contentLen)
            return false;

        if (header.info.contentLen > MAX_PACKET_CONTENT_SIZE)
            return false;

        return true;
    }

    uint16 GetContentLen() const
    {
        return header.info.contentLen;
    }

public:
    struct HeaderVal
    {
        uint16 envTag;
        uint16 contentLen;
    };
    union Header
    {
        HeaderVal info;
        char blob[4];
    };
    Header header;
    char*  conetnt;
};

}
