#pragma once

#include "Export.h"
#include "Singleton.hpp"

#include <memory>
#include <unordered_set>

namespace sh::core
{
    template<typename T>
    class TrackingAllocator : public Singleton<TrackingAllocator<T>>
    {
        friend class GC;
    private:
        std::unordered_set<T*> allocatedMemory;
    public:
        using ValueType = T;

        auto Allocate(size_t size) -> T*
        {
            T* ptr = reinterpret_cast<T*>(::operator new(size));
            allocatedMemory.insert(ptr);

            return ptr;
        }

        void DeAllocate(T* p) noexcept 
        {
            allocatedMemory.erase(p);
            ::operator delete(p);
        }

        bool IsHeapAllocated(const T* p) 
        {
            return allocatedMemory.find(const_cast<T*>(p)) != allocatedMemory.end();
        }
    };
}