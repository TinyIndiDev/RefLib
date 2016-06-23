#pragma once

#include <memory>
#include <vector>
#include <map>
#include "reflib_runable_threads.h"
#include "reflib_safelock.h"
#include "reflib_composit_id.h"

namespace RefLib
{

class GameNetObj;
class NetListener;
class NetWorkerServer;

class NetService
    : public RefLib::RunableThreads
{
public:
    NetService();
    virtual ~NetService();

    bool Initialize(unsigned port, uint32 maxCnt, uint32 concurrency);
    void Shutdown();

    bool Register(std::weak_ptr<GameNetObj> obj);

    HANDLE GetCompletionPort() const { return _comPort; }
    std::weak_ptr<GameNetObj> GetObj(const CompositId& id);

    bool AllocObj(const CompositId& id);
    bool FreeObj(const CompositId& id);

protected:
    // run by thread
    virtual unsigned Run() override;

private:
    typedef std::map<uint32, std::shared_ptr<GameNetObj>> FREE_NET_OBJS;
    typedef std::vector<std::shared_ptr<GameNetObj>> GAME_NET_OBJS;

    SafeLock _freeLock;
    FREE_NET_OBJS _freeObjs;
    GAME_NET_OBJS _objs;

    std::unique_ptr<NetListener> _netListener;
    std::unique_ptr<NetWorkerServer> _netWorker;

    uint32 _maxCnt;
    HANDLE _comPort;
};

} // namespace RefLib