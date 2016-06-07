// SimpleCS.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"

#include "reflib_net_listener.h"
#include "reflib_net_worker_server.h"

#pragma comment(lib,"WS2_32")

using namespace RefLib;

int main()
{
    unsigned maxUser = 3000;
    unsigned port = 5150;

    NetWorkerServer workerServer;
    NetListener netListener;

    if (!g_network.Initialize())
        return -1;

    if (!netListener.Initialize(maxUser))
        return -1;

    if (!workerServer.Initialize())
        return -1;

    netListener.Listen(port);

    netListener.Shutdown();
    workerServer.Deactivate();

    return 0;
}
