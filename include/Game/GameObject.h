#pragma once

#include "Export.h"

#include "Object.h"
#include "Component/Component.h"
#include "Component/Transform.h"
#include "World.h"

#include "Core/GarbageCollection.h"
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
		PROPERTY(components)
		std::vector<Component*> components;

		std::string objName;

		bool bEnable;
		bool bInit;
	public:
		World& world;
		PROPERTY(transform)
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
		SH_GAME_API void BeginUpdate() override;
		SH_GAME_API void Update() override;
		SH_GAME_API void LateUpdate() override;

		SH_GAME_API void Destroy();

		SH_GAME_API void SetActive(bool b);
		SH_GAME_API void SetName(const std::string& name);

		SH_GAME_API auto GetComponents() const -> const std::vector<Component*>&;

		SH_GAME_API void AddComponent(Component* component);
	public:
		template<typename T>
		auto AddComponent() -> std::enable_if_t<IsComponent<T>::value, T*>
		{
			components.push_back(core::SObject::Create<T>());
			components.back()->SetOwner(*this);
			components.back()->SetActive(true);
			return static_cast<T*>(components.back());
		}
		template<typename T>
		auto GetComponent() const -> T*
		{
			for (Component* component : components)
			{
				if (T* casting = sh::core::reflection::Cast<T>(component); casting != nullptr)
					return casting;
			}
			return nullptr;
		}
	};
}