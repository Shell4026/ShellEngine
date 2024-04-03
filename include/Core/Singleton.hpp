#pragma once

#include "NonCopyable.h"

#include <memory>
#include <mutex>
namespace sh::core {
	template<typename T>
	class Singleton : public NonCopyable {
	private:
		static std::unique_ptr<T> instance;
	protected:
		Singleton() = default;
	public:
		static auto GetInstance()->T*;
	};

	template<typename T>
	std::unique_ptr<T> Singleton<T>::instance = nullptr;

	template<typename T>
	auto Singleton<T>::GetInstance() -> T*
	{
		std::call_once(std::once_flag(), []() {
			instance = std::make_unique<T>();
		});
		return instance.get();
	}
}