#pragma once

#include "reflib_type_def.h"
#include <limits>

namespace RefLib
{

#define PACKET_ENVELOP_TAG      0xffaa
#define PACKET_HEADER_SIZE      PacketHeaderObj::GetHeaderSize()
#define MAX_PACKET_CONTENT_SIZE (MAX_PACKET_SIZE - PACKET_HEADER_SIZE)

class PacketHeaderObj
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
		return (header.info.contentLen <= MAX_PACKET_CONTENT_SIZE);
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
};

}
