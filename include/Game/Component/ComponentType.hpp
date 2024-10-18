#pragma once
#include <type_traits>
#include <memory>

#include "Core/SObject.h"

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
		using Type = T;

		auto Create(GameObject& owner) -> Component* override
		{
			return core::SObject::Create<T>(owner);
		}
	};
}//namespace
