#pragma once

#include "reflib_non_copyable.h"
#include <vector>
#include <atomic>

namespace RefLib
{

class RunableThreads : public NonCopyable
{
public:
    RunableThreads();
    virtual ~RunableThreads();

    void Activate();
    void Deactivate();

    bool IsActive() const { return _activated; }

    virtual void OnDeactivated() {}

    // call by thread
    virtual unsigned RunByThread();

protected:
    bool CreateThreads(unsigned threadCnt);
    bool CreateThreads(unsigned threadCnt, unsigned(__stdcall *ThreadProc)(void *));

    // call by thread
    virtual unsigned Run() { return 0; };

private:
    static unsigned __stdcall ThreadProc(void* param);

    void Resume();
    unsigned Join();

    bool OnTimeout();
    bool OnTerminated();

    std::atomic<bool> _activated;
    std::vector<HANDLE> _hThreads;
};

} // namespace RefLib
