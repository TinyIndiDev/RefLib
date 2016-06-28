#include "stdafx.h"

#include <windows.h>
#include <process.h>
#include <exception>
#include <assert.h>

#include "reflib_runable_threads.h"

#define TIMEOUT_CNT_LIMIT 5

namespace RefLib
{

unsigned __stdcall RunableThreads::ThreadProc(void* param)
{
    RunableThreads* workers = static_cast<RunableThreads*>(param);

    return workers->RunByThread();
}

RunableThreads::RunableThreads()
    : _activated(false)
{
}

RunableThreads::~RunableThreads()
{
    try
    {
        for (auto handle : _hThreads)
        {
            CloseHandle(handle);
        }
    }
    catch (...)
    {
        // CloseHandle can throw exception under debugger.
        // suppress any exception; dtors should never throw
    }
}

bool RunableThreads::CreateThreads(unsigned threadCnt)
{
    REFLIB_ASSERT_RETURN_VAL_IF_FAILED(threadCnt <= MAXIMUM_WAIT_OBJECTS, 
        "Maxium thread count cannot exceed MAXIMUM_WAIT_OBJECTS", false);

    for (unsigned i = 0; i < threadCnt; ++i)
    {
        HANDLE hThread = reinterpret_cast<HANDLE>(
            _beginthreadex(0, 0, ThreadProc, static_cast<void*>(this), CREATE_SUSPENDED, nullptr));
        if (hThread == 0)
        {
            DebugPrint("failed to create thread");
            return false;
        }
        _hThreads.push_back(hThread);
    }

    return true;
}

bool RunableThreads::CreateThreads(unsigned threadCnt, unsigned(__stdcall *ThreadProc)(void *))
{
    REFLIB_ASSERT_RETURN_VAL_IF_FAILED(threadCnt <= MAXIMUM_WAIT_OBJECTS,
        "Maxium thread count cannot exceed MAXIMUM_WAIT_OBJECTS", false);

    for (unsigned i = 0; i < threadCnt; ++i)
    {
        HANDLE hThread = reinterpret_cast<HANDLE>(
            _beginthreadex(0, 0, ThreadProc, static_cast<void*>(this), CREATE_SUSPENDED, nullptr));
        if (hThread == 0)
        {
            DebugPrint("failed to create thread");
            return false;
        }
        _hThreads.push_back(hThread);
    }

    return true;
}

void RunableThreads::Activate()
{
    bool expected = false;
    if (_activated.compare_exchange_weak(expected, true))
    {
        Resume();
    }
}

void RunableThreads::Deactivate()
{
    bool expected = true;
    if (_activated.compare_exchange_weak(expected, false))
    {
        Join();
    }
}

void RunableThreads::Resume()
{
    for (auto element : _hThreads)
    {
        ResumeThread(element);
    }
}

unsigned RunableThreads::Join()
{
    unsigned ret = 0;
    bool quit = false;

    do {
        ret = ::WaitForMultipleObjects(static_cast<DWORD>(_hThreads.size()), &(_hThreads.at(0)), TRUE, THREAD_TIMEOUT_IN_MSEC);
        switch (ret)
        {
        case WAIT_OBJECT_0:
            quit = OnTerminated();
            break;
        case WAIT_TIMEOUT:
            quit = OnTimeout();
            break;
        }
    } while (!quit);

    OnDeactivated();

    return ret;
}

bool RunableThreads::OnTimeout()
{
    DebugPrint("WaitForMultipleObjects: timeout");
    return (_activated == false);
}

bool RunableThreads::OnTerminated()
{
    DebugPrint("WaitForMultipleObjects: terminated");
    return true;
}

unsigned RunableThreads::RunByThread()
{
    return Run();
}

} // namespace RefLib
