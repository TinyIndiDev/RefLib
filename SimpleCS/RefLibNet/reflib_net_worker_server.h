#pragma once

#include "reflib_runable_threads.h"
#include "reflib_net_profiler.h"
#include <memory>

namespace RefLib
{

class NetSocketBase;
class NetCompletionOP;
class NetService;

class NetWorkerServer
    : public RunableThreads
    , public NetProfiler
{
public:
    NetWorkerServer(NetService* container);
    virtual ~NetWorkerServer() {}

    virtual bool Initialize(unsigned int concurrency);
    void Shutdown();

    virtual void OnDeactivated() override;

protected:
    // run by thread
    virtual unsigned Run() override;
    void HandleIO(NetSocketBase* sock, NetCompletionOP* bufObj, DWORD bytesTransfered, int error);

private:
    HANDLE _comPort;
    NetService* _container;
};

} // namespace RefLib
