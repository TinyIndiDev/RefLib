#pragma once

namespace RefLib
{

template <typename T> class Singleton
{
public:
    Singleton()
    {
        REFLIB_ASSERT(!_singleton, "Duplicated singleton.");

        // Pointer manipulating must be long to support 64bit machine.
        INT_PTR offset = (INT_PTR)(T*)1 - (INT_PTR)(Singleton <T>*)(T*)1;
        _singleton = (T*)((INT_PTR)this + offset);
    }
    ~Singleton(void)
    {
        REFLIB_ASSERT(_singleton, "Singleton released somewhere already.");
        _singleton = 0;
    }
    static T& GetSingleton(void)
    {
        REFLIB_ASSERT(_singleton, "No singleton exists.");
        return (*_singleton);
    }
    static T* GetSingletonPtr(void)
    {
        return (_singleton);
    }

private:
    static T* _singleton;
};

} // namespace RefLib

template <typename T> T* RefLib::Singleton<T>::_singleton = 0;
