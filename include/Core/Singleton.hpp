#pragma once

#include "NonCopyable.h"
#include "Export.h"

#include <vector>
#include <atomic>
#include <mutex>
#include <utility>

namespace sh::core 
{
	/// @brief DLL간 공유를 위해 쓰이는 인터페이스.
	class ISingleton
	{
	private:
		struct InstanceInfo
		{
			uint64_t hash;
			std::size_t size;
			void* ptr;
		};
		SH_CORE_API static std::mutex mu;
		SH_CORE_API static std::vector<InstanceInfo> instance;
	protected:
		struct Result
		{
			void* ptr;
			bool bNeedInit;
		};

		SH_CORE_API static auto CreateInstance(uint64_t hash, std::size_t size) -> Result;
		SH_CORE_API static void DeleteInstance(uint64_t hash, std::size_t size);
	};

	/// @brief 스레드에 안전한 싱글톤 클래스.
	/// @tparam T 타입
	/// @tparam shareDLL DLL간 메모리 영역을 공유할지
	template<typename T, bool shareDLL = true>
	class Singleton : private ISingleton, public INonCopyable
	{
	private:
		static std::atomic<T*> instance;
		static std::mutex mu;
	protected:
		Singleton() {}
	public:
		/// @brief 싱글톤 인스턴스를 생성하거나 가져온다.
		/// 
		/// @details 같은 dll내 멀티 스레드 환경에서는 lock을 쓰지 않고 memory_order를 이용하여 lock-free로 구현.
		/// @details 다른 dll내 멀티 스레드 환경에서는 ISingleton의 CreateInstance내부에서 락을 써서 구현.
		/// @tparam ...Args 가변 인자 타입
		/// @param ...args 생성자에 전달 할 인자. 첫 초기화 시에만 사용된다.
		/// @return 객체 포인터
		template<typename ...Args>
		static auto GetInstance(Args&&... args)->T*;
		static void Destroy();
	};

	template<typename T>
	class Singleton<T, false> : public INonCopyable
	{
	private:
		static std::atomic<T*> instance;
	protected:
		Singleton() {}
	public:
		template<typename ...Args>
		static auto GetInstance(Args&&... args) -> T*;
		static void Destroy();
	};

	template<typename T, bool shareDLL>
	std::atomic<T*> Singleton<T, shareDLL>::instance{};
	template<typename T, bool shareDLL>
	std::mutex Singleton<T, shareDLL>::mu{};

	template<typename T>
	std::atomic<T*> Singleton<T, false>::instance{};

	template<typename T, bool shareDLL>
	template<typename ...Args>
	auto Singleton<T, shareDLL>::GetInstance(Args&&... args) -> T*
	{
		// instance 변수는 DLL마다 고유의 주소를 가지고 있다는 점을 기억
		T* instancePtr = instance.load(std::memory_order::memory_order_acquire);
		if (instancePtr == nullptr)
		{
			// 여기까진 여러 스레드가 동시에 접근 할 가능성이 존재
			std::lock_guard<std::mutex> lockGuard{ mu };
			// 여기부턴 한 스레드만 접근 가능
			instancePtr = instance.load(std::memory_order::memory_order_relaxed);
			if (instancePtr == nullptr)
			{
				if constexpr (shareDLL)
				{
					// DLL간 공유 하기 원한다면 ISingleton 클래스에서 이전에 만들어진 객체가 있었는지를 찾는다.
					uint64_t hash = typeid(T).hash_code();
					Result result = CreateInstance(hash, sizeof(T));
					void* resultPtr = result.ptr;
					if (result.bNeedInit)
						new (resultPtr) T(std::forward<Args>(args)...);
					instancePtr = reinterpret_cast<T*>(resultPtr);
				}
				else
					instancePtr = new T{ std::forward<Args>(args)... };
				instance.store(instancePtr, std::memory_order::memory_order_release);
			}	
		}
		return instancePtr;
	}

	/// @brief 생성 돼 있는 싱글톤 인스턴스의 메모리를 해제한다.
	/// @tparam T 타입
	template<typename T, bool shareDLL>
	void Singleton<T, shareDLL>::Destroy()
	{
		T* instancePtr = instance.load(std::memory_order::memory_order_acquire);
		if (instancePtr != nullptr)
		{
			std::lock_guard<std::mutex> guard{ mu };
			instancePtr = instance.load(std::memory_order::memory_order_relaxed);
			if (instancePtr != nullptr)
			{
				if constexpr (shareDLL)
				{
					instancePtr->~T();
					DeleteInstance(typeid(T).hash_code(), sizeof(T));
				}
				else
					delete instancePtr;
				instance.store(nullptr, std::memory_order::memory_order_release);
			}
		}
	}
}//namespace