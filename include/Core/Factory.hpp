#pragma once
#include "Singleton.hpp"

#include <memory>
#include <unordered_map>
#include <string>
#include <functional>
namespace sh::core
{
	template<typename T>
	class Factory : public Singleton<Factory<T>>
	{
		friend Singleton<Factory<T>>;
	public:
		using AssetFactoryFn = std::function<std::unique_ptr<T>()>;
	private:
		std::unordered_map<std::string, AssetFactoryFn> factories;
	protected:
		Factory() = default;
	public:
		virtual ~Factory() = default;

		void Register(const std::string& name, AssetFactoryFn fn);
		void UnRegister(const std::string& name);
		auto Create(const std::string& name) const -> std::unique_ptr<T>;
	};
	template<typename T>
	inline void Factory<T>::Register(const std::string& name, AssetFactoryFn fn)
	{
		factories.insert_or_assign(name, fn);
	}
	template<typename T>
	inline void Factory<T>::UnRegister(const std::string& name)
	{
		factories.erase(name);
	}
	template<typename T>
	inline auto Factory<T>::Create(const std::string& name) const -> std::unique_ptr<T>
	{
		auto it = factories.find(name);
		if (it == factories.end())
			return nullptr;
		return it->second();
	}
}//namespace