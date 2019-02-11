#pragma once

#include "reflib_runable_threads.h"
#include "reflib_net_profiler.h"
#include <memory>

namespace RefLib
{

struct NetCompletionOP;
class NetSocketBase;
class NetService;

class NetWorker
    : public RunableThreads
    , public NetProfiler
{
public:
	NetWorker(NetService* container);
    virtual ~NetWorker() {}

    virtual bool Initialize(unsigned int concurrency);

    virtual void OnDeactivated() override;

protected:
    // run by thread
    virtual void Run() override;
    void HandleIO(NetSocketBase* sock, OVERLAPPED* lpOverlapped, DWORD bytesTransfered, int error);

private:
    HANDLE _comPort;
    NetService* _container;
};

} // namespace RefLib
