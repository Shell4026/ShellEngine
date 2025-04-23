#include "ComponentModule.h"

namespace sh::game
{
	auto ComponentModule::GetComponents() -> std::unordered_map<std::string, std::unique_ptr<IComponentType>>&
	{
		return components;
	}
	auto ComponentModule::GetComponents() const -> const std::unordered_map<std::string, std::unique_ptr<IComponentType>>&
	{
		return components;
	}
	auto ComponentModule::GetComponent(std::string_view name) const -> IComponentType*
	{
		auto it = components.find(std::string{ name });
		if (it == components.end())
			return nullptr;
		return it->second.get();
	}
	SH_GAME_API void ComponentModule::RegisterWaitingComponents()
	{
		for (auto& componentInfo : waitingComponents)
			components.insert_or_assign(componentInfo.name, std::move(componentInfo.componentType));
		waitingComponents.clear();
	}
	SH_GAME_API auto ComponentModule::GetWaitingComponents() const -> const std::vector<ComponentInfo>&
	{
		return waitingComponents;
	}
	SH_GAME_API void ComponentModule::DestroyComponent(const std::string& name)
	{
		auto it = components.find(name);
		if (it != components.end())
			components.erase(it);
	}
}