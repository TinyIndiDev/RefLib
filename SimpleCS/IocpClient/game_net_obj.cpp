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

    std::cout << "GameNetObj::OnConnected" << std::endl;

    std::cout << "Sending packet" << std::endl;

    for (int i = 0; i < 10; ++i)
    {
        std::string msg = "hello";
        Send((char*)msg.c_str(), (uint16)msg.length() + 1);

        Sleep(200);
    }
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
        std::string msg(buffer->GetData(), buffer->GetDataLen());
        std::cout << msg << std::endl;

        g_memoryPool.FreeBuffer(buffer);
    }

    return true;
}
