#pragma once

#include <memory>
#include <vector>
#include <map>
#include "reflib_runable_threads.h"
#include "reflib_safelock.h"
#include "reflib_composit_id.h"

namespace RefLib
{

class NetObj;
class NetConnectionProxy;
class NetWorker;

class NetService
    : public RefLib::RunableThreads
{
public:
    NetService();
    virtual ~NetService();

    bool InitServer(uint32 maxCnt, uint32 concurrency);
    bool InitClient(uint32 maxCnt, uint32 concurrency);
    void StartListen(unsigned port);
    void Shutdown();

    bool AddListening(std::weak_ptr<NetObj> obj);
    bool Connect(const std::string& ipStr, uint32 port, std::weak_ptr<NetObj> obj);

    HANDLE GetCompletionPort() const { return _comPort; }
    std::weak_ptr<NetObj> GetNetObj(const CompositId& id);

    bool AllocNetObj(const CompositId& id);
    bool FreeNetObj(const CompositId& id);

    void OnTerminated(NetServiceChildType childType);

protected:
    bool RegisterNetObj(std::weak_ptr<NetObj> obj);

    // run by thread
    virtual void Run() override;

private:
    typedef std::map<uint32, std::shared_ptr<NetObj>> FREE_NET_OBJS;
    typedef std::vector<std::shared_ptr<NetObj>> GAME_NET_OBJS;

    GAME_NET_OBJS _objs;
    FREE_NET_OBJS _freeObjs;

    std::unique_ptr<NetConnectionProxy> _netConnectionProxy;
    std::unique_ptr<NetWorker> _netWorker;

    uint32 _maxCnt;
    HANDLE _comPort;

    SafeLock _freeLock;
};

} // namespace RefLib