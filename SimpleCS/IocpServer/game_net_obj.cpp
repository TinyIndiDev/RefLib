#include "stdafx.h"

#include <iostream>
#include "game_net_obj.h"
#include "reflib_memory_block.h"
#include "reflib_memory_pool.h"

GameNetObj::GameNetObj(std::weak_ptr<RefLib::NetService> container)
    : NetObj(container)
{
}

GameNetObj::~GameNetObj()
{
}

void GameNetObj::OnConnected()
{
	NetObj::OnConnected();
}

void GameNetObj::OnDisconnected()
{
	NetObj::OnDisconnected();
}

bool GameNetObj::OnRecvPacket()
{
    RefLib::MemoryBlock* buffer = nullptr;

    while (buffer = PopRecvPacket())
    {
        std::string msg(buffer->GetData(), buffer->GetDataLen()+1);
        std::cout << msg << std::endl;

        Send(buffer->GetData(), buffer->GetDataLen() + 1);

        g_memoryPool.FreeBuffer(buffer);
    }

    return true;
}
