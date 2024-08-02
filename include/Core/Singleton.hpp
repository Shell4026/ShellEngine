#pragma once

#include "NonCopyable.h"

#include <atomic>
#include <mutex>

namespace sh::core {
	template<typename T>
	class Singleton : public INonCopyable {
	private:
		static std::atomic<T*> instance;
		static std::mutex mu;
	protected:
		Singleton() = default;
	public:
		static auto GetInstance()->T*;
		static void Destroy();
	};

	template<typename T>
	std::atomic<T*> Singleton<T>::instance{};
	template<typename T>
	std::mutex Singleton<T>::mu{};

	template<typename T>
	auto Singleton<T>::GetInstance() -> T*
	{
		T* instancePtr = instance.load(std::memory_order::memory_order_acquire);
		if (instancePtr == nullptr)
		{
			std::lock_guard<std::mutex> lockGuard{ mu };
			instancePtr = instance.load(std::memory_order::memory_order_relaxed);
			if (instancePtr == nullptr)
			{
				instancePtr = new T;
				instance.store(instancePtr, std::memory_order::memory_order_release);
			}	
		}
		return instancePtr;
	}

	template<typename T>
	void Singleton<T>::Destroy()
	{
		T* instancePtr = instance.load(std::memory_order::memory_order_acquire);
		if (instancePtr != nullptr)
		{
			std::lock_guard<std::mutex> guard{ mu };
			instancePtr = instance.load(std::memory_order::memory_order_relaxed);
			if (instancePtr != nullptr)
			{
				delete instancePtr;
				instance.store(nullptr, std::memory_order::memory_order_release);
			}
		}
	}
}