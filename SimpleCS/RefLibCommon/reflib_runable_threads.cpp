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
        _hThreads.insert(hThread);
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
        _hThreads.insert(hThread);
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

void RunableThreads::Join()
{
    while (!_hThreads.empty()) 
    {
        std::set<HANDLE> threads_left;

        for (std::set<HANDLE>::iterator cur_thread = _hThreads.begin(), last = _hThreads.end(); cur_thread != last; ++cur_thread)
        {
            DWORD rc = ::WaitForSingleObject(*cur_thread, THREAD_TIMEOUT_IN_MSEC);
            if (rc == WAIT_OBJECT_0) 
            {
                ::CloseHandle(*cur_thread); // necessary with _beginthreadex
            }
            else if (rc == WAIT_TIMEOUT) 
            {
                threads_left.insert(*cur_thread); // wait again
            }
            else 
            {
                // this shouldn't happen... try to close the handle and hope
                // for the best!
                ::CloseHandle(*cur_thread); // necessary with _beginthreadex
            }
        }
        std::swap(threads_left, _hThreads);
    }

    OnDeactivated();
}

unsigned RunableThreads::RunByThread()
{
    while (IsActive())
    {
        Run();
    }

    return 0;
}

} // namespace RefLib
