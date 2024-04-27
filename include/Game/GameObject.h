#pragma once

#include "Export.h"

#include "SObject.h"
#include "Component/Component.h"
#include "World.h"

#include "Core/Reflaction.hpp"
#include "Core/Util.h"

#include <vector>
#include <memory>

namespace sh::game
{
	class GameObject : public ISObject
	{
		SCLASS(GameObject)
	private:
		std::vector<std::unique_ptr<Component>> components;

		std::string objName;

		bool bEnable;

		bool bInit : 1;
	public:
		const bool& activeSelf;
		const std::string& name;
		World& world;
	public:
		SH_GAME_API GameObject(World& world, const std::string& name);
		SH_GAME_API ~GameObject();

		SH_GAME_API void Awake() override;
		SH_GAME_API void Start() override;
		SH_GAME_API void OnEnable() override;
		SH_GAME_API void Update() override;
		SH_GAME_API void LateUpdate() override;
		
		SH_GAME_API void SetActive(bool b);
		SH_GAME_API void SetName(const std::string& name);

		SH_GAME_API auto GetComponents() const -> const std::vector<std::unique_ptr<Component>>&;
	public:
		template<typename T>
		auto AddComponent() -> std::enable_if_t<IsComponent<T>::value, T*>
		{
			components.push_back(std::make_unique<T>(*this));
			return static_cast<T*>(components[components.size() - 1].get());
		}
		template<typename T>
		auto GetComponent() const -> T*
		{
			for (auto& component : components)
			{
				if (T* casting = sh::core::Util::Cast<T>(&component) != nullptr)
					return casting;
			}
			return nullptr;
		}
	};
}