#pragma once

#include "NonCopyable.h"

namespace sh::core {
	template<typename T>
	class Singleton : public INonCopyable {
	private:
		static T* instance;
	protected:
		Singleton() = default;
	public:
		static auto GetInstance()->T*;
		static void Destroy();
	};

	template<typename T>
	T* Singleton<T>::instance = nullptr;

	template<typename T>
	auto Singleton<T>::GetInstance() -> T*
	{
		if (instance == nullptr)
			instance = new T;

		return instance;
	}

	template<typename T>
	void Singleton<T>::Destroy()
	{
		delete instance;
	}
}