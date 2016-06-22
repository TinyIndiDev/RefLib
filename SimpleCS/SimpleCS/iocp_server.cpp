// SimpleCS.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"

#include "reflib_net_service.h"
#include "reflib_net_api.h"
#include "reflib_game_net_obj.h"

#pragma comment(lib,"WS2_32")

using namespace RefLib;

int main()
{
    unsigned maxUser = 3000;
    unsigned port = 5150;

    auto netService = std::make_shared<NetService>();

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    if (sysInfo.dwNumberOfProcessors > NETWORK_MAX_COMPLETION_THREAD_COUNT)
        sysInfo.dwNumberOfProcessors = NETWORK_MAX_COMPLETION_THREAD_COUNT;

    if (!g_network.Initialize())
        return -1;

    if (!netService->Initialize(port, maxUser, sysInfo.dwNumberOfProcessors))
        return -1;

    for (int i = 0; i < 10; ++i)
    {
        auto obj = std::make_shared<GameNetObj>(netService);
        netService->Register(obj);
    }

    netService->Shutdown();

    return 0;
}
