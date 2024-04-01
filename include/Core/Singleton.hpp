#pragma once

#include "NonCopyable.h"
#include "Util.h"

#include <memory>
#include <fmt/core.h>
namespace sh::core {
	template<typename T>
	class Singleton : public NonCopyable {
	private:
		static std::shared_ptr<T> instance;
	protected:
		Singleton() {}
	public:
		static auto GetInstance()->std::shared_ptr<T>;
	};

	template<typename T>
	std::shared_ptr<T> Singleton<T>::instance = std::shared_ptr<T>();

	template<typename T>
	auto Singleton<T>::GetInstance() -> std::shared_ptr<T>
	{
		if (instance.get() == nullptr)
		{
			if (Util::IsDebug())
				fmt::print("Instance Create: {}\n", typeid(T).name());
			instance = std::make_shared<T>();
		}
		return instance;
	}
}