#pragma once

#include "Export.h"

#include "Component/ComponentType.hpp"

#include "Core/Singleton.hpp"
#include "Core/SContainer.hpp"

#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>

#define REGISTER_COMPONENT(component) \
ComponentModule::GetInstance()->RegisterComponent<component>(#component)

namespace sh::game
{
	class ComponentModule : public core::Singleton<ComponentModule>
	{
	private:
		core::SHashMap<std::string, std::unique_ptr<IComponentType>> components;
	public:
		template<typename T>
		void RegisterComponent(std::string_view name, std::string&& group);

		SH_GAME_API auto GetComponents() -> core::SHashMap<std::string, std::unique_ptr<IComponentType>>&;
		SH_GAME_API auto GetComponents() const -> const core::SHashMap<std::string, std::unique_ptr<IComponentType>>&;
		SH_GAME_API auto GetComponent(std::string_view name) const -> IComponentType*;
	};

	template<typename T>
	inline void ComponentModule::RegisterComponent(std::string_view name, std::string&& group)
	{
		if (group.size() == 0)
			group = name;
		else
		{
			group.push_back('/');
			group += name;
		}
		components.insert({ std::move(group), std::make_unique<ComponentType<T>>()});
	}
}