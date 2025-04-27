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
		/// @brief 컴포넌트를 생성한다.
		/// @param owner 게임 오브젝트
		/// @return 컴포넌트 포인터
		virtual auto Create(GameObject& owner) -> Component* = 0;
		/// @brief 복사 후 생성한다. 타입이 다르면 Create()하고 같다.
		/// @param other 복사 할 컴포넌트
		/// @return 컴포넌트 포인터, 복사 실패 시 nullptr
		virtual auto Clone(GameObject& owner, const Component& other) -> Component* = 0;
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

		auto Clone(GameObject& owner, const Component& other) -> Component* override
		{
			auto ptr = core::SObject::Create<T>(owner);
			ptr->SetName(name);

			if (T::GetStaticType() != other.GetType())
				return ptr;

			core::Json serialization = other.Serialize();
			if (serialization.empty())
				return nullptr;

			if (serialization.contains("uuid"))
			{
				serialization["uuid"] = ptr->GetUUID().ToString();
			}
			ptr->Deserialize(serialization);
			return ptr;
		}
	};
}//namespace
