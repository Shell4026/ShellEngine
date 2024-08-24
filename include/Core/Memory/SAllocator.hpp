﻿#pragma once

#include "MemoryPool.hpp"

#include "../Reflection.hpp"
#include "../SObject.h"

#include <memory>
#include <iostream>

namespace sh::core::memory
{
	/// @brief 엔진에서 사용하는 할당자
	/// @tparam T 타입
	/// @tparam size 한번에 할당 된 메모리 갯수
	template<typename T, std::size_t size = 128>
	class SAllocator : private MemoryPool<T, size>
	{
	private:
		using Super = MemoryPool<T, size>;

#if defined(_WIN32)
		SAllocator* copyAllocator = nullptr;
		std::allocator<T>* rebindAllocator = nullptr;
#endif
	public:
		using size_type = std::size_t ;
		using difference_type = std::ptrdiff_t ;
		using pointer = T*;
		using const_pointer = const T*;
		using reference = T&;
		using const_reference = const T&;
		using value_type = T;

		template <class U>
		struct rebind
		{
			typedef SAllocator<U, size> other;
		};
	public:
#if defined(_WIN32)
		SAllocator() = default;

		SAllocator(SAllocator& allocator) :
			copyAllocator(&allocator)
		{
		}

		template <class U>
		SAllocator(const SAllocator<U, size>& other)
		{
			if (!std::is_same<T, U>::value)
				rebindAllocator = new std::allocator<T>();
		}

		~SAllocator()
		{
			delete rebindAllocator;
		}
#endif

		auto allocate(size_type n, const void* hint = 0) -> pointer
		{
#if defined(_WIN32)
			if (copyAllocator)
				return copyAllocator->allocate(n, hint);

			if (rebindAllocator)
				return rebindAllocator->allocate(n, hint);
#endif

			if (n != 1 || hint)
				throw std::bad_alloc();

			return Super::Allocate();
		}

		void deallocate(pointer p, size_type n)
		{
#if defined(_WIN32)
			if (copyAllocator) 
			{
				copyAllocator->deallocate(p, n);
				return;
			}

			if (rebindAllocator) 
			{
				rebindAllocator->deallocate(p, n);
				return;
			}
#endif

			Super::DeAllocate(p);
		}

		void construct(pointer p, const_reference val)
		{
			new (p) T(val);
		}
		template< class U, class... Args >
		void construct(U* ptr, Args&&... args)
		{
			new((void*)ptr) U(std::forward<Args>(args)...);
		}

		void destroy(pointer p)
		{
			p->~T();
		}
	};
}