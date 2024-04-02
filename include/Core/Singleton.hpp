#pragma once

#include "NonCopyable.h"

#include <memory>
namespace sh::core {
	template<typename T>
	class Singleton : public NonCopyable {
	private:
		static std::unique_ptr<T> instance;
	protected:
		Singleton() {}
	public:
		static auto GetInstance()->T*;
	};

	template<typename T>
	std::unique_ptr<T> Singleton<T>::instance = nullptr;

	template<typename T>
	auto Singleton<T>::GetInstance() -> T*
	{
		if (instance.get() == nullptr)
		{
			instance = std::make_unique<T>();
		}
		return instance.get();
	}
}