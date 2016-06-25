#include "stdafx.h"

#include "game_net_obj.h"

GameNetObj::GameNetObj(std::weak_ptr<RefLib::NetService> container)
    : NetObj(container)
{
}

GameNetObj::~GameNetObj()
{
}

void GameNetObj::OnRecvPacket()
{
    // TODO: treat parsed packet
}
