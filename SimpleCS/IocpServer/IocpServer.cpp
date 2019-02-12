// IocpServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include "reflib_net_service.h"
#include "game_net_obj.h"

#pragma comment(lib,"WS2_32")

using namespace RefLib;

SYSTEM_INFO getSystemInfo()
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	if (sysInfo.dwNumberOfProcessors > NETWORK_MAX_COMPLETION_THREAD_COUNT)
		sysInfo.dwNumberOfProcessors = NETWORK_MAX_COMPLETION_THREAD_COUNT;

	return sysInfo;
}

int main()
{
    std::cout << "Press enter to quit: " << std::endl;

    unsigned port = 5150;
    unsigned maxConn = 3000;

    auto netService = std::make_shared<NetServerService>();
    if (!netService->Initialize(maxConn, getSystemInfo().dwNumberOfProcessors))
        return -1;

    for (size_t i = 0; i < maxConn; ++i)
    {
        auto obj = std::make_shared<GameNetObj>(netService);
		if (!netService->AddListeningObj(obj))
		{
			return -1;
		}
    }

    netService->StartListen(port);

    getchar();

    netService->Shutdown();

    return 0;
}
