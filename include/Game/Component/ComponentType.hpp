#pragma once

#include "Component.h"

#include <type_traits>
#include <memory>

namespace sh::game
{
	struct IComponentType
	{
		virtual auto Create() -> std::unique_ptr<Component> = 0;
	};
	template<typename T, typename IsComponent = std::enable_if_t<std::is_base_of_v<Component, T>>>
	struct ComponentType : IComponentType
	{
		using Type = T;

		auto Create() -> std::unique_ptr<Component> override
		{
			return std::unique_ptr<Component>(new T);
		}
	};
}//namespace
