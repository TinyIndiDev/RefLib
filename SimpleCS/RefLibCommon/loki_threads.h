#pragma once

////////////////////////////////////////////////////////////////////////////////
///  Simplified version of Loki Threads.h
////////////////////////////////////////////////////////////////////////////////

#define LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL ::Loki::ClassLevelLockable

#ifndef LOKI_DEFAULT_MUTEX
#define LOKI_DEFAULT_MUTEX ::Loki::Mutex
#endif

#define LOKI_THREADS_MUTEX(x)           CRITICAL_SECTION (x);
#define LOKI_THREADS_MUTEX_INIT(x)      ::InitializeCriticalSection (x)
#define LOKI_THREADS_MUTEX_DELETE(x)    ::DeleteCriticalSection (x)
#define LOKI_THREADS_MUTEX_LOCK(x)      ::EnterCriticalSection (x)
#define LOKI_THREADS_MUTEX_UNLOCK(x)    ::LeaveCriticalSection (x)
#define LOKI_THREADS_LONG               LONG

#define LOKI_THREADS_ATOMIC_FUNCTIONS                                   \
        static IntType AtomicIncrement(volatile IntType& lval)          \
        { return InterlockedIncrement(&const_cast<IntType&>(lval)); }   \
                                                                        \
        static IntType AtomicDecrement(volatile IntType& lval)          \
        { return InterlockedDecrement(&const_cast<IntType&>(lval)); }   \
                                                                        \
        static void AtomicAssign(volatile IntType& lval, IntType val)   \
        { InterlockedExchange(&const_cast<IntType&>(lval), val); }      \
                                                                        \
        static void AtomicAssign(IntType& lval, volatile IntType& val)  \
        { InterlockedExchange(&lval, val); }

namespace Loki
{

////////////////////////////////////////////////////////////////////////////////
///  A simple and portable Mutex.  A default policy class for locking objects.
////////////////////////////////////////////////////////////////////////////////

class Mutex
{
public:
    Mutex()
    {
        LOKI_THREADS_MUTEX_INIT(&mtx_);
    }
    ~Mutex()
    {
        LOKI_THREADS_MUTEX_DELETE(&mtx_);
    }
    void Lock()
    {
        LOKI_THREADS_MUTEX_LOCK(&mtx_);
    }
    void Unlock()
    {
        LOKI_THREADS_MUTEX_UNLOCK(&mtx_);
    }
private:
    /// Copy-constructor not implemented.
    Mutex(const Mutex &);
    /// Copy-assignement operator not implemented.
    Mutex & operator = (const Mutex &);
    LOKI_THREADS_MUTEX(mtx_)
};

////////////////////////////////////////////////////////////////////////////////
///  Implementation of the ThreadingModel policy used by various classes
///  Implements a object-level locking scheme
////////////////////////////////////////////////////////////////////////////////

template < class Host, class MutexPolicy = LOKI_DEFAULT_MUTEX >
class ObjectLevelLockable
{
    mutable MutexPolicy mtx_;

public:
    ObjectLevelLockable() : mtx_() {}

    ObjectLevelLockable(const ObjectLevelLockable&) : mtx_() {}

    ~ObjectLevelLockable() {}

    class Lock;
    friend class Lock;

    ///  Lock class to lock on object level
    class Lock
    {
    public:

        /// Lock object
        explicit Lock(const ObjectLevelLockable& host) : host_(host)
        {
            host_.mtx_.Lock();
        }

        /// Lock object
        explicit Lock(const ObjectLevelLockable* host) : host_(*host)
        {
            host_.mtx_.Lock();
        }

        /// Unlock object
        ~Lock()
        {
            host_.mtx_.Unlock();
        }

    private:
        /// private by design of the object level threading
        Lock();
        Lock(const Lock&);
        Lock& operator=(const Lock&);
        const ObjectLevelLockable& host_;
    };

    typedef volatile Host VolatileType;

    typedef LOKI_THREADS_LONG IntType;

    LOKI_THREADS_ATOMIC_FUNCTIONS
};

////////////////////////////////////////////////////////////////////////////////
///  Implementation of the ThreadingModel policy used by various classes
///  Implements a class-level locking scheme

template <class Host, class MutexPolicy = LOKI_DEFAULT_MUTEX >
class ClassLevelLockable
{
    struct Initializer
    {
        bool init_;
        MutexPolicy mtx_;

        Initializer() : init_(false), mtx_()
        {
            init_ = true;
        }

        ~Initializer()
        {
            assert(init_);
        }
    };

    static Initializer initializer_;

public:
    class Lock;
    friend class Lock;

    ///  Lock class to lock on class level
    class Lock
    {
    public:
        /// Lock class
        Lock()
        {
            assert(initializer_.init_);
            initializer_.mtx_.Lock();
        }

        /// Lock class
        explicit Lock(const ClassLevelLockable&)
        {
            assert(initializer_.init_);
            initializer_.mtx_.Lock();
        }

        /// Lock class
        explicit Lock(const ClassLevelLockable*)
        {
            assert(initializer_.init_);
            initializer_.mtx_.Lock();
        }

        /// Unlock class
        ~Lock()
        {
            assert(initializer_.init_);
            initializer_.mtx_.Unlock();
        }

    private:
        Lock(const Lock&);
        Lock& operator=(const Lock&);
    };

    typedef volatile Host VolatileType;

    typedef LOKI_THREADS_LONG IntType;

    LOKI_THREADS_ATOMIC_FUNCTIONS
};

template < class Host, class MutexPolicy >
typename ClassLevelLockable< Host, MutexPolicy >::Initializer
ClassLevelLockable< Host, MutexPolicy >::initializer_;

};

