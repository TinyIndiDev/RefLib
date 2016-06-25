// IocpClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "reflib_net_service.h"
#include "reflib_net_api.h"
#include "game_net_obj.h"

#pragma comment(lib,"WS2_32")

using namespace RefLib;

int main()
{
    std::string ipStr = "127.0.0.1";
    uint32 port = 5150;

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    if (sysInfo.dwNumberOfProcessors > NETWORK_MAX_COMPLETION_THREAD_COUNT)
        sysInfo.dwNumberOfProcessors = NETWORK_MAX_COMPLETION_THREAD_COUNT;

    if (!g_network.Initialize())
        return -1;

    auto netService = std::make_shared<NetService>();

    uint32 maxConn = 1;
    if (!netService->InitClient(maxConn, sysInfo.dwNumberOfProcessors))
        return -1;

    auto obj = std::make_shared<GameNetObj>(netService);
    netService->Register(obj);

    obj->Connect(ipStr, port);

    //send packet
    std::string msg = "hello";
    obj->Send((char*)msg.c_str(), (uint16)msg.length()+1);

    netService->Shutdown();

    return 0;
}

