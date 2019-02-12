// IocpClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include "reflib_net_service.h"
#include "game_net_obj.h"

#pragma comment(lib,"WS2_32")

using namespace RefLib;

int main()
{
    std::cout << "Press enter to quit: " << std::endl;

    std::string ipStr = "127.0.0.1";
    uint32 port = 5150;
    uint32 maxConn = 1;

    auto netService = std::make_shared<NetClientService>();
	if (!netService->Initialize(maxConn, 1))
		return -1;

	auto obj = std::make_shared<GameNetObj>(netService);
	if (!netService->Connect(ipStr, port, obj))
        return -1;

    getchar();

    netService->Shutdown();

    return 0;
}

