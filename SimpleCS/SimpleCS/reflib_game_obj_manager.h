#pragma once

#include <vector>
#include <deque>
#include "reflib_runable_threads.h"
#include "reflib_safelock.h"
#include "reflib_composit_id.h"

namespace RefLib
{

class GameObj;

class GameObjMgr
    : public RefLib::RunableThreads
{
public:
    GameObjMgr();
    virtual ~GameObjMgr();

    bool Initialize(uint32 maxCnt, uint32 concurrency);

    GameObj* GetGameObj();
    void FreeGameObj(const CompositId& id);
    void FreeGameObj(GameObj* obj);

protected:
    // run by thread
    virtual unsigned Run() override;

private:
    typedef std::vector<GameObj*> GAME_OBJS;

    std::deque<CompositId> _freeIds;
    RefLib::SafeLock _freeLock;

    GAME_OBJS _gameObjs;
    uint32 _maxCnt;

    HANDLE _comPort;
};

} // namespace RefLib