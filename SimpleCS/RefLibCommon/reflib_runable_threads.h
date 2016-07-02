#pragma once

#include "reflib_non_copyable.h"
#include <set>
#include <atomic>

namespace RefLib
{

class RunableThreads : public NonCopyable
{
public:
    RunableThreads();
    virtual ~RunableThreads();

    void Activate();
    void Join();
    void Deactivate();

    bool IsActive() const { return _activated; }

    virtual void OnDeactivated() {}

    // call by thread
    virtual unsigned RunByThread();

protected:
    bool CreateThreads(unsigned threadCnt);
    bool CreateThreads(unsigned threadCnt, unsigned(__stdcall *ThreadProc)(void *));

    // call by thread
    virtual void Run() {};

private:
    static unsigned __stdcall ThreadProc(void* param);

    void Resume();

    std::atomic<bool> _activated;
    std::set<HANDLE> _hThreads;
};

} // namespace RefLib
