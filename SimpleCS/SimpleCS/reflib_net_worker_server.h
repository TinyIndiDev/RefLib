#pragma once

#include "reflib_runable_threads.h"
#include "reflib_net_profiler.h"
#include <memory>

namespace RefLib
{

class NetSocketBase;
class NetCompletionOP;

class NetWorkerServer
    : public RunableThreads
    , public NetProfiler
{
public:
    NetWorkerServer();
    virtual ~NetWorkerServer() {}

    virtual bool Initialize();

protected:
    // run by thread
    virtual unsigned Run() override;
    void HandleIO(NetSocketBase* sock, NetCompletionOP* bufObj, DWORD bytesTransfered, int error);

private:
    HANDLE _completionPort;
};

} // namespace RefLib
