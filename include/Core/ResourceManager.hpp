#pragma once

#include "Resource.h"
#include "Singleton.hpp"

#include <type_traits>
#include <vector>
#include <queue>
#include <memory>

#include <iostream>
namespace sh::core
{
	template<typename T, typename IsResource = std::enable_if_t<std::is_base_of_v<Resource, T> && !std::is_pointer_v<T>>>
	class ResourceManager : public Singleton<ResourceManager<T>>
	{
	private:
		std::queue<int> empty;
		std::vector<std::unique_ptr<T>> objs;
	public:
		ResourceManager();
		void Add(T* obj);
		void Add(std::unique_ptr<T>&& obj);

		auto Get(unsigned int handle) -> T*;
		auto GetNextHandle() -> int;
	};

	template<typename T, typename IsResource>
	inline ResourceManager<T, IsResource>::ResourceManager()
	{
		objs.push_back(nullptr);
	}

	template<typename T, typename IsResource>
	inline void ResourceManager<T, IsResource>::Add(std::unique_ptr<T>&& obj)
	{
		std::cout << this << '\n';
		if (empty.empty())
			objs.push_back(std::move(obj));
		else
		{
			int idx = empty.front();
			objs[idx] = std::move(obj);
			empty.pop();
		}
	}

	template<typename T, typename IsResource>
	inline void ResourceManager<T, IsResource>::Add(T* obj)
	{
		if(empty.empty())
			objs.push_back(std::make_unique<T>(obj));
		else
		{
			int idx = empty.front();
			objs[idx] = std::make_unique<T>(obj);
			empty.pop();
		}
	}

	template<typename T, typename IsResource>
	inline auto ResourceManager<T, IsResource>::GetNextHandle() -> int
	{
		if(empty.empty())
			return objs.size();
		return empty.front();
	}

	template<typename T, typename IsResource>
	inline auto ResourceManager<T, IsResource>::Get(unsigned int handle) -> T*
	{
		std::cout << this << '\n';
		return objs[handle].get();
	}
}