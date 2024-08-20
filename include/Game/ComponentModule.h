#pragma once

#include "Export.h"

#include "Component/ComponentType.hpp"

#include "Core/Singleton.hpp"

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
		std::unordered_map<std::string, std::unique_ptr<IComponentType>> components;
	public:
		template<typename T>
		void RegisterComponent(std::string_view name);

		SH_GAME_API auto GetComponents() -> std::unordered_map<std::string, std::unique_ptr<IComponentType>>&;
		SH_GAME_API auto GetComponents() const -> const std::unordered_map<std::string, std::unique_ptr<IComponentType>>&;
		SH_GAME_API auto GetComponent(std::string_view name) const -> IComponentType*;
	};

	template<typename T>
	inline void ComponentModule::RegisterComponent(std::string_view name)
	{
		components.insert({ std::string{name}, std::make_unique<ComponentType<T>>() });
	}
}