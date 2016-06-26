#pragma once

////////////////////////////////////////////////////////////////////////////////
///  Simplified version of Loki Singleton.h
////////////////////////////////////////////////////////////////////////////////

#include "loki_threads.h"

#define LOKI_C_CALLING_CONVENTION_QUALIFIER __cdecl   

namespace Loki
{

typedef void (LOKI_C_CALLING_CONVENTION_QUALIFIER *atexit_pfn_t)();

////////////////////////////////////////////////////////////////////////////////
///  Implementation of the CreationPolicy used by SingletonHolder
///  Creates objects using a straight call to the new operator 
////////////////////////////////////////////////////////////////////////////////
template <class T> struct CreateUsingNew
{
    static T* Create()
    {
        return new T;
    }

    static void Destroy(T* p)
    {
        delete p;
    }
};

////////////////////////////////////////////////////////////////////////////////
///  Implementation of the CreationPolicy used by SingletonHolder
///  Creates objects using a custom allocater.
///  Usage: e.g. CreateUsing<std::allocator>::Allocator
////////////////////////////////////////////////////////////////////////////////
template<template<class> class Alloc>
struct CreateUsing
{
    template <class T>
    struct Allocator
    {
        static Alloc<T> allocator;

        static T* Create()
        {
            return new (allocator.allocate(1)) T;
        }

        static void Destroy(T* p)
        {
            p->~T();
            allocator.deallocate(p, 1);
        }
    };
};

////////////////////////////////////////////////////////////////////////////////
///  Implementation of the CreationPolicy used by SingletonHolder
///  Creates objects using a call to std::malloc, followed by a call to the 
///  placement new operator
////////////////////////////////////////////////////////////////////////////////
template <class T> struct CreateUsingMalloc
{
    static T* Create()
    {
        void* p = std::malloc(sizeof(T));
        if (!p) return 0;
        return new(p) T;
    }

    static void Destroy(T* p)
    {
        p->~T();
        std::free(p);
    }
};

////////////////////////////////////////////////////////////////////////////////
///  Implementation of the CreationPolicy used by SingletonHolder
///  Creates an object in static memory
///  Implementation is slightly nonportable because it uses the MaxAlign trick 
///  (an union of all types to ensure proper memory alignment). This trick is 
///  nonportable in theory but highly portable in practice.
////////////////////////////////////////////////////////////////////////////////
template <class T> struct CreateStatic
{

#ifdef _MSC_VER
#pragma warning( push ) 
#pragma warning( disable : 4121 )
    // alignment of a member was sensitive to packing
#endif // _MSC_VER

    union MaxAlign
    {
        char t_[sizeof(T)];
        short int shortInt_;
        int int_;
        long int longInt_;
        float float_;
        double double_;
        long double longDouble_;
        struct Test;
        int Test::* pMember_;
        int (Test::*pMemberFn_)(int);
    };

#ifdef _MSC_VER
#pragma warning( pop )
#endif // _MSC_VER

    static T* Create()
    {
        static MaxAlign staticMemory_;
        return new(&staticMemory_) T;
    }

    static void Destroy(T* p)
    {
        p->~T();
    }
};

////////////////////////////////////////////////////////////////////////////////
///  Implementation of the LifetimePolicy used by SingletonHolder
///  Schedules an object's destruction as per C++ rules
///  Forwards to std::atexit
////////////////////////////////////////////////////////////////////////////////
template <class T>
struct DefaultLifetime
{
    static void ScheduleDestruction(T*, atexit_pfn_t pFun)
    {
        std::atexit(pFun);
    }

    static void OnDeadReference()
    {
        throw std::logic_error("Dead Reference Detected");
    }
};

////////////////////////////////////////////////////////////////////////////////
///  Provides Singleton amenities for a type T
///  To protect that type from spurious instantiations, 
///  you have to protect it yourself.
///  
///  \param CreationPolicy Creation policy, default: CreateUsingNew
///  \param LifetimePolicy Lifetime policy, default: DefaultLifetime,
///  \param ThreadingModel Threading policy, 
///                         default: LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL
////////////////////////////////////////////////////////////////////////////////
template
<
    typename T,
    template <class> class CreationPolicy = CreateUsingNew,
    template <class> class LifetimePolicy = DefaultLifetime,
    template <class, class> class ThreadingModel = LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL,
    class MutexPolicy = LOKI_DEFAULT_MUTEX
>
class SingletonHolder
{
public:
    ///  Type of the singleton object
    typedef T ObjectType;

    ///  Returns a reference to singleton object
    static T& Instance();

private:
    // Helpers
    static void MakeInstance();
    static void LOKI_C_CALLING_CONVENTION_QUALIFIER DestroySingleton();

    // Protection
    SingletonHolder();

    // Data
    typedef typename ThreadingModel<T*, MutexPolicy>::VolatileType PtrInstanceType;
    static PtrInstanceType pInstance_;
    static bool destroyed_;
};

////////////////////////////////////////////////////////////////////////////////
// SingletonHolder's data

template
<
    class T,
    template <class> class C,
    template <class> class L,
    template <class, class> class M,
    class X
>
typename SingletonHolder<T, C, L, M, X>::PtrInstanceType
SingletonHolder<T, C, L, M, X>::pInstance_;

template
<
    class T,
    template <class> class C,
    template <class> class L,
    template <class, class> class M,
    class X
>
bool SingletonHolder<T, C, L, M, X>::destroyed_;

////////////////////////////////////////////////////////////////////////////////
// SingletonHolder::Instance

template
<
    class T,
    template <class> class CreationPolicy,
    template <class> class LifetimePolicy,
    template <class, class> class ThreadingModel,
    class MutexPolicy
>
inline T& SingletonHolder<T, CreationPolicy,
    LifetimePolicy, ThreadingModel, MutexPolicy>::Instance()
{
    if (!pInstance_)
    {
        MakeInstance();
    }
    return *pInstance_;
}

////////////////////////////////////////////////////////////////////////////////
// SingletonHolder::MakeInstance (helper for Instance)

template
<
    class T,
    template <class> class CreationPolicy,
    template <class> class LifetimePolicy,
    template <class, class> class ThreadingModel,
    class MutexPolicy
>
void SingletonHolder<T, CreationPolicy,
    LifetimePolicy, ThreadingModel, MutexPolicy>::MakeInstance()
{
    typename ThreadingModel<SingletonHolder, MutexPolicy>::Lock guard;
    (void)guard;

    if (!pInstance_)
    {
        if (destroyed_)
        {
            destroyed_ = false;
            LifetimePolicy<T>::OnDeadReference();
        }
        pInstance_ = CreationPolicy<T>::Create();
        LifetimePolicy<T>::ScheduleDestruction(pInstance_,
            &DestroySingleton);
    }
}

template
<
    class T,
    template <class> class CreationPolicy,
    template <class> class L,
    template <class, class> class M,
    class X
>
void LOKI_C_CALLING_CONVENTION_QUALIFIER
SingletonHolder<T, CreationPolicy, L, M, X>::DestroySingleton()
{
    assert(!destroyed_);
    CreationPolicy<T>::Destroy(pInstance_);
    pInstance_ = 0;
    destroyed_ = true;
}


////////////////////////////////////////////////////////////////////////////////
///  Convenience template to implement a getter function for a singleton object.
///  Often needed in a shared library which hosts singletons.
////////////////////////////////////////////////////////////////////////////////

#ifndef LOKI_SINGLETON_EXPORT
#define LOKI_SINGLETON_EXPORT
#endif

template<class T>
class LOKI_SINGLETON_EXPORT Singleton
{
public:
    static T& Instance();
};

} // namespace Loki

/// Convenience macro for the definition of the static Instance member function
/// Put this macro called with a SingletonHolder typedef into your cpp file.

#define LOKI_SINGLETON_INSTANCE_DEFINITION(SHOLDER)                     \
namespace Loki                                                          \
{                                                                       \
    template<>                                                          \
    SHOLDER::ObjectType&  Singleton<SHOLDER::ObjectType>::Instance()    \
    {                                                                   \
        return SHOLDER::Instance();                                     \
    }                                                                   \
}
