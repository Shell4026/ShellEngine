#pragma once

#include <atomic>
#include <thread>
namespace sh::core
{
	/// @brief 공유 데이터를 빠르게 처리 할 수 있는 곳에서 잠그는데 좋은 스핀락 클래스
	class SpinLock
	{
	private:
		/// @brief atomic_bool이 어느 장치에서나 락프리라고 보장하지 않기 때문에 atomic_flag를 선택 할 수도 있다.
		/// @brief C++20에서는 atomic_flag의 test함수로 인해 atomic_flag만 사용해도 된다.
		union AtomicFlag
		{
			std::atomic_flag flag;
			std::atomic_bool boolean;

			AtomicFlag()
			{
				if constexpr (std::atomic_bool::is_always_lock_free)
				{
					new (this) std::atomic_bool{ false };
				}
				else
					new (this) std::atomic_flag{};
			}
		} atomicFlag;
	public:
		inline void Lock() noexcept
		{
			if constexpr (std::atomic_bool::is_always_lock_free)
			{
				while (!TryLock())
				{
					while (atomicFlag.boolean.load(std::memory_order::memory_order_relaxed))
						std::this_thread::yield();
				}
			}
			else
			{
				while (atomicFlag.flag.test_and_set(std::memory_order::memory_order_acquire))
					std::this_thread::yield();
			}
		}
		inline auto TryLock() noexcept -> bool
		{
			if constexpr (std::atomic_bool::is_always_lock_free)
				return !atomicFlag.boolean.exchange(true, std::memory_order::memory_order_acquire);
			else
				return !atomicFlag.flag.test_and_set(std::memory_order::memory_order_acquire);
		}
		inline void UnLock() noexcept
		{
			if constexpr (std::atomic_bool::is_always_lock_free)
				atomicFlag.boolean.store(false, std::memory_order::memory_order_release);
			else
				atomicFlag.flag.clear(std::memory_order::memory_order_release);
		}
	};
}//namespace