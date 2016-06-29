// IocpServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include "reflib_net_service.h"
#include "reflib_net_api.h"
#include "game_net_obj.h"

#pragma comment(lib,"WS2_32")

using namespace RefLib;

void RunService()
{
    getchar();
}

int main()
{
    std::cout << "Press enter to quit: " << std::endl;

    unsigned port = 5150;
    unsigned maxConn = 3000;

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    if (sysInfo.dwNumberOfProcessors > NETWORK_MAX_COMPLETION_THREAD_COUNT)
        sysInfo.dwNumberOfProcessors = NETWORK_MAX_COMPLETION_THREAD_COUNT;

    if (!g_network.Initialize())
        return -1;

    auto netService = std::make_shared<NetService>();
    if (!netService->InitServer(maxConn, sysInfo.dwNumberOfProcessors))
        return -1;

    for (int i = 0; i < 10; ++i)
    {
        auto obj = std::make_shared<GameNetObj>(netService);
        netService->AddListening(obj);
    }

    netService->StartListen(port);

    RunService();

    netService->Shutdown();

    return 0;
}
