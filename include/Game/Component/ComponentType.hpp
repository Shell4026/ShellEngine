#pragma once

#include "Component.h"

#include "Core/SObject.h"

#include <type_traits>
#include <memory>

namespace sh::game
{
	struct IComponentType
	{
		virtual auto Create() -> Component* = 0;
	};
	template<typename T, typename IsComponent = std::enable_if_t<std::is_base_of_v<Component, T>>>
	struct ComponentType : IComponentType
	{
		using Type = T;

		auto Create() -> Component* override
		{
			return core::SObject::Create<T>();
		}
	};
}//namespace
