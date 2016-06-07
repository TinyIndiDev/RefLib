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

    virtual void Activate();
    virtual void Deactivate();

    // call by thread
    virtual unsigned RunByThread();

protected:
    bool CreateThreads(unsigned threadCnt);

    virtual bool OnTimeout();
    virtual bool OnTerminated();
    virtual bool OnAbandoned();
    virtual bool OnFailed();

    // call by thread
    virtual unsigned Run() { return 0; };

private:
    static unsigned __stdcall ThreadProc(void* param);

    virtual void Resume();
    virtual unsigned Join();

    std::atomic<bool> _activated;
    std::vector<HANDLE> _hThreads;
};

} // namespace RefLib
