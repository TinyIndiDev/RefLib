// SimpleCS.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"

#include "reflib_net_listener.h"
#include "reflib_net_worker_server.h"
#include "reflib_game_obj_manager.h"

#pragma comment(lib,"WS2_32")

using namespace RefLib;

int main()
{
    unsigned maxUser = 3000;
    unsigned port = 5150;

    GameObjMgr gameObjMgr;
    NetWorkerServer workerServer;
    NetListener netListener(&gameObjMgr);

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    if (sysInfo.dwNumberOfProcessors > NETWORK_MAX_COMPLETION_THREAD_COUNT)
        sysInfo.dwNumberOfProcessors = NETWORK_MAX_COMPLETION_THREAD_COUNT;

    if (!g_network.Initialize())
        return -1;

    if (!gameObjMgr.Initialize(maxUser, sysInfo.dwNumberOfProcessors))
        return -1;

    if (!netListener.Initialize(maxUser))
        return -1;

    if (!workerServer.Initialize(sysInfo.dwNumberOfProcessors))
        return -1;

    netListener.Listen(port);

    netListener.Shutdown();
    workerServer.Deactivate();

    return 0;
}
