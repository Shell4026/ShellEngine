#pragma once

#include "Export.h"

#include "Object.h"
#include "Component/Component.h"
#include "Component/Transform.h"
#include "World.h"

#include "Core/Reflection.hpp"
#include "Core/Util.h"

#include <vector>
#include <memory>

namespace sh::game
{
	class GameObject : public IObject
	{
		SCLASS(GameObject)
	private:
		std::vector<std::unique_ptr<Component>> components;
		

		std::string objName;

		bool bEnable;

		bool bInit : 1;
	public:
		World& world;
		Transform* transform;

		const bool& activeSelf;
		const std::string& name;
	public:
		SH_GAME_API GameObject(World& world, const std::string& name);
		SH_GAME_API GameObject(GameObject&& other) noexcept;
		SH_GAME_API ~GameObject();

		SH_GAME_API void Awake() override;
		SH_GAME_API void Start() override;
		SH_GAME_API void OnEnable() override;
		SH_GAME_API void Update() override;
		SH_GAME_API void LateUpdate() override;
		
		SH_GAME_API void Destroy();

		SH_GAME_API void SetActive(bool b);
		SH_GAME_API void SetName(const std::string& name);

		SH_GAME_API auto GetComponents() const -> const std::vector<std::unique_ptr<Component>>&;

		SH_GAME_API void AddComponent(std::unique_ptr<Component>&& component);
	public:
		template<typename T>
		auto AddComponent() -> std::enable_if_t<IsComponent<T>::value, T*>
		{
			components.push_back(std::make_unique<T>());
			components.back()->SetOwner(*this);
			components.back()->SetGC(world.gc);
			return static_cast<T*>(components.back().get());
		}
		template<typename T>
		auto GetComponent() const -> T*
		{
			for (auto& component : components)
			{
				if (T* casting = sh::core::reflection::Cast<T>(&component) != nullptr)
					return casting;
			}
			return nullptr;
		}
	};
}