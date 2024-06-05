﻿#include "ComponentModule.h"

namespace sh::game
{
	auto ComponentModule::GetComponents() -> std::unordered_map<std::string, std::unique_ptr<Component>>&
	{
		return components;
	}
	auto ComponentModule::GetComponents() const -> const std::unordered_map<std::string, std::unique_ptr<Component>>&
	{
		return components;
	}
	auto ComponentModule::GetComponent(std::string_view name) const -> Component*
	{
		auto it = components.find(std::string{ name });
		if (it == components.end())
			return nullptr;
		return it->second.get();
	}
}