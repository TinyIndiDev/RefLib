#include "stdafx.h"

#include <algorithm>
#include "reflib_net_service.h"
#include "reflib_net_connection.h"
#include "reflib_net_connection_manager.h"
#include "reflib_net_listener.h"
#include "reflib_net_connector.h"
#include "reflib_net_worker_server.h"
#include "reflib_net_obj.h"

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

bool NetService::InitServer(unsigned port, uint32 maxCnt, uint32 concurrency)
{
    _comPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, (ULONG_PTR)nullptr, concurrency);
    if (!_comPort)
    {
        DebugPrint("CreateIoCompletionPort failed: %d", GetLastError());
        return false;
    }

    _maxCnt = maxCnt;
    _objs.resize(maxCnt);

    _netConnectionProxy = std::make_unique<NetListener>(this);
    if (!_netConnectionProxy->Initialize(maxCnt))
        return false;

    _netWorker = std::make_unique<NetWorkerServer>(this);
    if (!_netWorker->Initialize(concurrency))
        return false;

    if (!CreateThreads(concurrency))
        return false;

    RunableThreads::Activate();

    _netConnectionProxy->Listen(port);

    return true;
}

bool NetService::InitClient(uint32 maxCnt, uint32 concurrency)
{
    _comPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, (ULONG_PTR)nullptr, concurrency);
    if (!_comPort)
    {
        DebugPrint("CreateIoCompletionPort failed: %d", GetLastError());
        return false;
    }

    _maxCnt = maxCnt;
    _objs.resize(maxCnt);

    _netConnectionProxy = std::make_unique<NetConnector>(this);
    if (!_netConnectionProxy->Initialize(maxCnt))
        return false;

    _netWorker = std::make_unique<NetWorkerServer>(this);
    if (!_netWorker->Initialize(concurrency))
        return false;

    if (!CreateThreads(concurrency))
        return false;

    RunableThreads::Activate();

    return true;
}

bool NetService::AddListening(std::weak_ptr<NetObj> obj)
{
    return RegisterToListener(obj);
}

bool NetService::Connect(const std::string& ipStr, uint32 port, std::weak_ptr<NetObj> obj)
{
    if (!RegisterToConnector(obj))
        return false;

    return _netConnectionProxy->Connect(ipStr, port, obj);
}

bool NetService::RegisterToListener(std::weak_ptr<NetObj> obj)
{
    auto p = obj.lock();
    if (!p) return false;

    auto con = _netConnectionProxy->RegisterCon().lock();
    if (!con) return false;

    if (p->Initialize(con))
    {
        con->RegisterParent(p);

        SafeLock::Owner lock(_freeLock);

        uint32 slot = static_cast<uint32>(_freeObjs.size());
        _freeObjs.emplace(slot, p);

        return true;
    }

    return false;
}

bool NetService::RegisterToConnector(std::weak_ptr<NetObj> obj)
{
    auto p = obj.lock();
    if (!p) return false;

    auto con = _netConnectionProxy->RegisterCon().lock();
    if (!con) return false;

    if (p->Initialize(con))
    {
        con->RegisterParent(p);

        SafeLock::Owner lock(_freeLock);

        uint32 slot = static_cast<uint32>(_freeObjs.size());
        _freeObjs.emplace(slot, p);

        return true;
    }

    return false;
}

std::weak_ptr<NetObj> NetService::GetNetObj(const CompositId& id)
{
    if (id.GetSlotId() >= _maxCnt)
        return std::weak_ptr<NetObj>();

    return _objs[id.GetSlotId()];
}

bool NetService::AllocNetObj(const CompositId& id)
{
    if (id.GetSlotId() >= _maxCnt)
        return false;

    SafeLock::Owner lock(_freeLock);

    auto it = _freeObjs.find(id.GetSlotId());
    if (it == _freeObjs.end())
        return false;

    std::shared_ptr<NetObj> p = it->second;

    _freeObjs.erase(it);
    _objs[id.GetSlotId()] = p;

    return true;
}

bool NetService::FreeNetObj(const CompositId& id)
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

    while (IsActive())
    {
        rc = GetQueuedCompletionStatus(
            _comPort,
            &bytesTransfered,
            &ulKey,
            &lpOverlapped,
            INFINITE);

        NetObj* obj = (NetObj*)ulKey;
        if (!obj)
        {
            DebugPrint("NetService::Run received shutdown signal");
            continue;
        }

        obj->OnRecvPacket();
    }    

    return 0;
}

void NetService::Shutdown()
{
    if (_netConnectionProxy)
    {
        DebugPrint("Shutdown NetConnectionProxy.");
        _netConnectionProxy->Shutdown();
    }
}

void NetService::OnTerminated(NetServiceChildType childType)
{
    switch (childType)
    {
    case NET_CTYPE_LISTENER:
    case NET_CYPTE_CONNECTOR:
        DebugPrint("Shutdown NetWorkerServer.");
        _netWorker->Shutdown();
        break;
    case NET_CTYPE_NETWORKER:
        DebugPrint("Shutdown NetService.");
        RunableThreads::Deactivate();
        ::PostQueuedCompletionStatus(_comPort, 0, NULL, NULL);
        break;
    default:
        break;
    }
}

} //namespace RefLib