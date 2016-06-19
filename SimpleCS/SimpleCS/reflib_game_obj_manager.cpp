#include "stdafx.h"
#include <algorithm>
#include "reflib_game_obj.h"
#include "reflib_game_obj_manager.h"

namespace RefLib
{

GameObjMgr::GameObjMgr()
    : _maxCnt(0)
    , _comPort(INVALID_HANDLE_VALUE)
{
}

GameObjMgr::~GameObjMgr()
{
    for (auto element : _gameObjs)
    {
        SAFE_DELETE(element);
    }
    _gameObjs.clear();

    if (_comPort)
    {
        CloseHandle(_comPort);
        _comPort = nullptr;
    }
}

bool GameObjMgr::Initialize(uint32 maxCnt, uint32 concurrency)
{
    _maxCnt = maxCnt;

    _comPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, (ULONG_PTR)nullptr, concurrency);
    if (_comPort)
    {
        DebugPrint("CreateIoCompletionPort failed: %d", GetLastError());
        return false;
    }

    if (!CreateThreads(concurrency))
    {
        CloseHandle(_comPort);
        return false;
    }

    _gameObjs.reserve(maxCnt);
    for (uint32 i = 0; i < maxCnt; ++i)
    {
        CompositId id(i, 0);

        _freeIds.push_back(id);
        _gameObjs[i] = new GameObj(id, _comPort);
    }

    RunableThreads::Activate();

    return true;
}

GameObj* GameObjMgr::GetGameObj()
{
    SafeLock::Owner lock(_freeLock);

    if (_freeIds.empty())
        return nullptr;

    CompositId id = _freeIds.front();
    _freeIds.pop_front();

    return _gameObjs[id.GetId()];
}

void GameObjMgr::FreeGameObj(GameObj* obj)
{
    if (obj)
    {
        FreeGameObj(obj->GetIndex());
    }
}

void GameObjMgr::FreeGameObj(const CompositId& id)
{
    if (id.GetId() >= _maxCnt)
        return;

    SafeLock::Owner lock(_freeLock);

    auto it = std::find_if(_freeIds.begin(), _freeIds.end(), [&id](const CompositId& item)
    {
        return item.GetId() == id.GetId();
    });
    REFLIB_ASSERT_RETURN_IF_FAILED(it != _freeIds.end(), "Same id exists in the freeIds deque.");

    GameObj* obj = _gameObjs[id.GetId()];
    REFLIB_ASSERT_RETURN_IF_FAILED(obj, "GameObj is null");

    obj->Reset();

    _freeIds.push_back(id);
}

unsigned GameObjMgr::Run()
{
    ULONG_PTR ulKey = 0;
    OVERLAPPED *lpOverlapped = nullptr;
    DWORD   bytesTransfered;
    int     rc;

    while (true)
    {
        rc = GetQueuedCompletionStatus(
            _comPort,
            &bytesTransfered,
            &ulKey,
            &lpOverlapped,
            INFINITE);

        GameObj* gameObj = (GameObj*)ulKey;
        if (gameObj)
        {
            gameObj->OnRecvPacket();
        }
    }    
    return 0;
}

} //namespace RefLib