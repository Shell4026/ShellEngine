#pragma once

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

		SAllocator* copyAllocator = nullptr;
		std::unique_ptr<std::allocator<T>> rebindAllocator = nullptr;
	public:
		using value_type = T;
		using pointer = T*;
		using const_pointer = const T*;
		using reference = T&;
		using const_reference = const T&;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		template <typename U>
		struct rebind
		{
			using other = SAllocator<U, size>;
		};
	public:
		SAllocator() = default;
		SAllocator(SAllocator& allocator) :
			copyAllocator(&allocator)
		{
		}
		template <class U>
		SAllocator(const SAllocator<U, size>& other)
		{
			if constexpr (!std::is_same<T, U>::value)
				rebindAllocator = std::make_unique<std::allocator<T>>();
		}
		~SAllocator() noexcept
		{
		}

		/// @brief 메모리 풀에서 메모리 할당
		/// @param n 쓰이지 않음
		/// @param hint 쓰이지 않음
		/// @return 할당된 메모리 포인터
		auto allocate(size_t n, const void* hint = nullptr) -> T*
		{
			if (copyAllocator)
				return copyAllocator->allocate(n, hint);
			if (rebindAllocator)
				return rebindAllocator->allocate(n, hint);

			return Super::Allocate();
		}
		
		void deallocate(T* ptr, std::size_t n) noexcept
		{
			if (copyAllocator) 
			{
				copyAllocator->deallocate(ptr, n);
				return;
			}

			if (rebindAllocator) 
			{
				rebindAllocator->deallocate(ptr, n);
				return;
			}

			Super::DeAllocate(ptr);
		}

		void construct(T* ptr, const_reference val)
		{
			//placement new - 포인터 위치에 val을 복사해서 T타입의 객체 생성
			new (ptr) T(val);
		}

		void destroy(T* ptr) noexcept
		{
			ptr->~T();
		}
	};
}