#include "stdafx.h"

#include <iostream>
#include "game_net_obj.h"

GameNetObj::GameNetObj(std::weak_ptr<RefLib::NetService> container)
    : NetObj(container)
{
}

GameNetObj::~GameNetObj()
{
}

void GameNetObj::OnConnected()
{
    std::cout << "GameNetObj::OnConnected" << std::endl;

    std::cout << "Sending packet" << std::endl;

    std::string msg = "hello";
    Send((char*)msg.c_str(), (uint16)msg.length() + 1);
}

void GameNetObj::OnDisconnected()
{

}

bool GameNetObj::OnRecvPacket()
{
    // TODO: treat parsed packet
    return true;
}
