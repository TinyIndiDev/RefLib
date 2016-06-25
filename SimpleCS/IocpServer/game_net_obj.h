#pragma once

#include "reflib_net_obj.h"

namespace RefLib
{
class NetConnection;
class NetService;
class MemoryBlock;
}

class GameNetObj : public RefLib::NetObj
{
public:
    GameNetObj(std::weak_ptr<RefLib::NetService> container);
    virtual ~GameNetObj();

    virtual void OnRecvPacket() override;
};

