// IocpClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include "reflib_net_service.h"
#include "reflib_net_api.h"
#include "game_net_obj.h"

#pragma comment(lib,"WS2_32")

using namespace RefLib;

void RunService(std::weak_ptr<GameNetObj> obj)
{
    auto p = obj.lock();

    std::cout << "Sending packet" << std::endl;

    std::string msg = "hello";
    p->Send((char*)msg.c_str(), (uint16)msg.length() + 1);
}

void CloseMessage()
{
    std::cout << "Press enter to quit: ";
    getchar();
}

int main()
{
    std::string ipStr = "127.0.0.1";
    uint32 port = 5150;
    uint32 maxConn = 1;

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    if (sysInfo.dwNumberOfProcessors > NETWORK_MAX_COMPLETION_THREAD_COUNT)
        sysInfo.dwNumberOfProcessors = NETWORK_MAX_COMPLETION_THREAD_COUNT;

    if (!g_network.Initialize())
        return -1;

    auto netService = std::make_shared<NetService>();
    if (!netService->InitClient(maxConn, sysInfo.dwNumberOfProcessors))
        return -1;

    auto obj = std::make_shared<GameNetObj>(netService);
    if (!netService->Connect(ipStr, port, obj))
        return -1;

    RunService(obj);

    netService->Shutdown();

    CloseMessage();

    return 0;
}

