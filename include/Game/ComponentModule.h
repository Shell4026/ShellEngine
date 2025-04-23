#pragma once
#include "Export.h"

#include "Component/ComponentType.hpp"

#include "Core/Singleton.hpp"
#include "Core/SContainer.hpp"
#include "Core/Reflection.hpp"

#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>
#include <vector>

#define REGISTER_COMPONENT(component) \
ComponentModule::GetInstance()->RegisterComponent<component>(#component)

namespace sh::game
{
	class ComponentModule : public core::Singleton<ComponentModule>
	{
		friend core::Singleton<ComponentModule>;
	public:
		struct ComponentInfo
		{
			std::string name;
			const core::reflection::STypeInfo& type;
			std::unique_ptr<IComponentType> componentType;
		};
	private:
		std::unordered_map<std::string, std::unique_ptr<IComponentType>> components;
		std::vector<ComponentInfo> waitingComponents;
	protected:
		ComponentModule() = default;
	public:
		template<typename T>
		void RegisterComponent(std::string_view name, std::string&& group);

		SH_GAME_API auto GetComponents() -> std::unordered_map<std::string, std::unique_ptr<IComponentType>>&;
		SH_GAME_API auto GetComponents() const -> const std::unordered_map<std::string, std::unique_ptr<IComponentType>>&;
		SH_GAME_API auto GetComponent(std::string_view name) const -> IComponentType*;

		SH_GAME_API void RegisterWaitingComponents();
		SH_GAME_API auto GetWaitingComponents() const -> const std::vector<ComponentInfo>&;
		SH_GAME_API void DestroyComponent(const std::string& name);
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
		std::string newName{ std::move(group) };
		if (GetComponent(newName) == nullptr)
			waitingComponents.push_back({ newName, T::GetStaticType(), std::make_unique<ComponentType<T>>(newName)});
	}
}