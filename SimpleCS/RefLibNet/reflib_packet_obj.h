#pragma once

#include "reflib_type_def.h"
#include <limits>

namespace RefLib
{

#define PACKET_ENVELOP_TAG 0xffaa

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

#define PACKET_HEADER_SIZE PacketObj::GetHeaderSize()
#define MAX_PACKET_CONTENT_SIZE ((1024)*(64) - PACKET_HEADER_SIZE)

}
