#pragma once
#include "Core/SObject.h"

#include <type_traits>
#include <memory>
#include <string>
#include <string_view>

namespace sh::game
{
	class Component;
	class GameObject;

	struct IComponentType
	{
		virtual auto Create(GameObject& owner) -> Component* = 0;
	};
	template<typename T, typename IsComponent = std::enable_if_t<std::is_base_of_v<Component, T>>>
	struct ComponentType : IComponentType
	{
		const std::string name;
		using Type = T;
		ComponentType(std::string_view name) :
			name(name) {}

		auto Create(GameObject& owner) -> Component* override
		{
			auto ptr = core::SObject::Create<T>(owner);
			ptr->SetName(name);
			return ptr;
		}
	};
}//namespace
