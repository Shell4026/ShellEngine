#pragma once

#include "MemoryPool.hpp"

#include <memory>
#include <iostream>
#include <type_traits>

namespace sh::core::memory
{
    /// @brief 엔진에서 사용하는 할당자
    /// @tparam T 타입
    /// @tparam size 한번에 할당 된 메모리 갯수
    template<typename T, std::size_t size = 8>
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
        MemoryPool<T, size> pool;
        bool useStdAllocator = false;
    public:
        SAllocator() = default;
        SAllocator(const SAllocator& other) :
            pool(other.pool), useStdAllocator(other.useStdAllocator)
        {
        }
        SAllocator(SAllocator&& other) noexcept : 
            pool(std::move(other.pool)), useStdAllocator(other.useStdAllocator)
        {
            other.useStdAllocator = false;
        }
        template <class U>
        SAllocator(const SAllocator<U, size>& other) : 
            pool(), useStdAllocator(!std::is_same_v<T, U>)
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
            return &this->pool == &other.pool;
        }

        template<typename U, std::size_t otherSize>
        bool operator!=(const SAllocator<U, otherSize>& other) const noexcept
        {
            return !(*this == other);
        }
    };
}
