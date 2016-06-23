#include "stdafx.h"

#include <algorithm>
#include "reflib_net_service.h"
#include "reflib_net_connection.h"
#include "reflib_net_connection_manager.h"
#include "reflib_net_listener.h"
#include "reflib_net_worker_server.h"
#include "reflib_game_net_obj.h"

namespace RefLib
{

NetService::NetService()
    : _maxCnt(0)
    , _comPort(INVALID_HANDLE_VALUE)
{
}

NetService::~NetService()
{
    if (_comPort)
    {
        CloseHandle(_comPort);
        _comPort = nullptr;
    }
}

bool NetService::Initialize(unsigned port, uint32 maxCnt, uint32 concurrency)
{
    _maxCnt = maxCnt;

    _comPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, (ULONG_PTR)nullptr, concurrency);
    if (!_comPort)
    {
        DebugPrint("CreateIoCompletionPort failed: %d", GetLastError());
        return false;
    }

    _objs.reserve(maxCnt);

    _netListener = std::make_unique<NetListener>();
    if (!_netListener->Initialize(maxCnt))
        return false;

    _netWorker = std::make_unique<NetWorkerServer>();
    if (!_netWorker->Initialize(concurrency))
        return false;

    if (!CreateThreads(concurrency))
        return false;

    RunableThreads::Activate();

    _netListener->Listen(port);

    return true;
}

bool NetService::Register(std::weak_ptr<GameNetObj> obj)
{
    auto p = obj.lock();
    if (!p)
        return false;

    if (!_netListener)
        return false;

    auto conn = _netListener->RegisterNetConnection().lock();
    if (!conn)
        return false;
    
    if (p->Initialize(conn))
    {
        conn->RegisterParent(p);

        SafeLock::Owner lock(_freeLock);

        uint32 slot = static_cast<uint32>(_freeObjs.size());
        _freeObjs.emplace(slot, p);

        return true;
    }

    return false;
}

std::weak_ptr<GameNetObj> NetService::GetObj(const CompositId& id)
{
    if (id.GetSlotId() >= _maxCnt)
        return std::weak_ptr<GameNetObj>();

    return _objs[id.GetSlotId()];
}

bool NetService::AllocObj(const CompositId& id)
{
    if (id.GetSlotId() >= _maxCnt)
        return false;

    SafeLock::Owner lock(_freeLock);

    auto it = _freeObjs.find(id.GetSlotId());
    if (it == _freeObjs.end())
        return false;

    std::shared_ptr<GameNetObj> p = it->second;

    _freeObjs.erase(it);
    _objs[id.GetSlotId()] = p;

    return true;
}

bool NetService::FreeObj(const CompositId& id)
{
    if (id.GetSlotId() >= _maxCnt)
        return false;

    auto p = _objs[id.GetSlotId()];
    if (!p)
        return false;

    _freeObjs.emplace(id.GetSlotId(), p);

    _objs[id.GetSlotId()].reset();

    return true;
}

unsigned NetService::Run()
{
    ULONG_PTR ulKey = 0;
    OVERLAPPED *lpOverlapped = nullptr;
    DWORD bytesTransfered;
    int rc;

    while (true)
    {
        rc = GetQueuedCompletionStatus(
            _comPort,
            &bytesTransfered,
            &ulKey,
            &lpOverlapped,
            INFINITE);

        GameNetObj* obj = (GameNetObj*)ulKey;
        if (obj)
        {
            obj->OnRecvPacket();
        }
    }    
    return 0;
}

void NetService::Shutdown()
{
    _netListener->Shutdown();
    _netWorker->Deactivate();
}

} //namespace RefLib