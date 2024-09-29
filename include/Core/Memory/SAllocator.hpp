#pragma once

#include "MemoryPool.hpp"

#include <memory>
#include <iostream>
#include <type_traits>

namespace sh::core::memory
{
    /// @brief 메모리 풀에서 메모리를 할당 받아오는 할당자.
    /// @tparam T 타입
    /// @tparam size 한번에 할당 된 메모리 갯수
    template<typename T, std::size_t size = 32>
    class SAllocator
    {
    public:
        using value_type = T;
        using pointer = T*;
        using const_pointer = const T*;
        using reference = T&;
        using const_reference = const T&;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        template <class U>
        struct rebind
        {
            typedef SAllocator<U, size> other;
        };
    private:
        bool useStdAllocator = false;

        inline static MemoryPool<T, size> pool;
    public:
        SAllocator() = default;
        SAllocator(const SAllocator& other) :
            useStdAllocator(other.useStdAllocator)
        {
        }
        SAllocator(SAllocator&& other) noexcept : 
            useStdAllocator(other.useStdAllocator)
        {
            other.useStdAllocator = false;
        }
        template <class U>
        SAllocator(const SAllocator<U, size>& other) : 
            useStdAllocator(!std::is_same_v<T, U>)
        {
        }

        ~SAllocator() = default;

        pointer allocate(size_type n)
        {
            if (useStdAllocator || n != 1)
            {
                return std::allocator<T>().allocate(n);
            }
            return pool.Allocate();
        }

        void deallocate(pointer p, size_type n)
        {
            if (useStdAllocator || n != 1)
            {
                std::allocator<T>().deallocate(p, n);
                return;
            }
            pool.DeAllocate(p);
        }

        template<typename U, std::size_t otherSize>
        bool operator==(const SAllocator<U, otherSize>& other) const noexcept
        {
            if constexpr (std::is_same_v<T, U>)
                return true;
            return false;
        }

        template<typename U, std::size_t otherSize>
        bool operator!=(const SAllocator<U, otherSize>& other) const noexcept
        {
            return !(*this == other);
        }
    };
}//namespace
