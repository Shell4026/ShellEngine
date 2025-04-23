#pragma once
#include "Export.h"

#include "IObject.h"
#include "Component/Component.h"
#include "Component/Transform.h"
#include "World.h"

#include "Core/Reflection.hpp"
#include "Core/Util.h"
#include "Core/Observer.hpp"

#include <vector>

namespace sh::game
{
	class GameObject : public core::SObject, public IObject
	{
		SCLASS(GameObject)
	private:
		PROPERTY(components)
		std::vector<Component*> components;
		bool bEnable;
		bool bInit;
	public:
		World& world;
		
		PROPERTY(transform)
		Transform* transform;

		const bool& activeSelf;

		mutable core::Observer<false, Component*> onComponentAdd;
		PROPERTY(hideInspector, core::PropertyOption::invisible)
		bool hideInspector = false;
		bool bNotSave = false;
	public:
		SH_GAME_API GameObject(World& world, const std::string& name);
		SH_GAME_API GameObject(GameObject&& other) noexcept;
		SH_GAME_API ~GameObject();

		SH_GAME_API void Destroy() override;

		SH_GAME_API void Awake() override;
		SH_GAME_API void Start() override;
		SH_GAME_API void OnEnable() override;
		SH_GAME_API void FixedUpdate() override;
		SH_GAME_API void BeginUpdate() override;
		SH_GAME_API void Update() override;
		SH_GAME_API void LateUpdate() override;

		SH_GAME_API void SetActive(bool b);

		SH_GAME_API auto GetComponents() const -> const std::vector<Component*>&;

		/// @brief 이미 만들어진 컴포넌트를 추가하는 함수. 
		/// 해당 컴포넌트의 gameObject가 이 gameObject와 다르면 추가 되지 않는다.
		/// @param component 컴포넌트 포인터
		SH_GAME_API void AddComponent(Component* component);

		SH_GAME_API auto Serialize() const -> core::Json override;
		SH_GAME_API void Deserialize(const core::Json& json) override;
	public:
		/// @brief 새 컴포넌트를 추가하는 함수
		/// @tparam T 컴포넌트 타입
		template<typename T>
		auto AddComponent() -> std::enable_if_t<IsComponent<T>::value, T*>
		{
			components.push_back(core::SObject::Create<T>(*this));

			auto ptr = components.back();
			ptr->SetActive(true);
			ptr->SetName(components.back()->GetType().name);

			onComponentAdd.Notify(ptr);

			return static_cast<T*>(ptr);
		}
		template<typename T>
		auto GetComponent() const -> T*
		{
			for (Component* component : components)
			{
				if (!core::IsValid(component))
					continue;
				if (component->GetType() == T::GetStaticType())
					return static_cast<T*>(component);
			}
			return nullptr;
		}
	};
}