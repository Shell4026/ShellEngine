#pragma once

#include "Export.h"

#include "Component/Component.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>

namespace sh::game
{
	class ComponentModule
	{
	private:
		std::unordered_map<std::string, std::unique_ptr<Component>> components;
	public:
		template<typename T>
		void RegisterComponent(std::string_view name);

		SH_GAME_API auto GetComponents() -> std::unordered_map<std::string, std::unique_ptr<Component>>&;
		SH_GAME_API auto GetComponents() const -> const std::unordered_map<std::string, std::unique_ptr<Component>>&;
		SH_GAME_API auto GetComponent(std::string_view name) const -> Component*;
	};

	template<typename T>
	inline void ComponentModule::RegisterComponent(std::string_view name)
	{
		components.insert({ std::string{name}, std::unique_ptr<Component>(new T{}) });
	}
}