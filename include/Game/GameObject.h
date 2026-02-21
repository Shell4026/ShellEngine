#pragma once
#include "Export.h"
#include "IObject.h"
#include "WorldEvents.hpp"
#include "Component/Component.h"
#include "Component/Transform.h"

#include "Core/Reflection.hpp"
#include "Core/Util.h"
#include "Core/Observer.hpp"
#include "Core/SContainer.hpp"

#include <vector>

namespace sh::game
{
	class World;
	class Collider;
	class GameObject : public core::SObject, public IObject
	{
		SCLASS(GameObject)
	public:
		struct CreateKey
		{
		private:
			friend World;
			CreateKey() = default;
		};
	public:
		SH_GAME_API GameObject(World& world, const std::string& name, CreateKey key);
		SH_GAME_API ~GameObject();

		SH_GAME_API auto operator=(const GameObject& other) -> GameObject&;

		SH_GAME_API void Destroy() override;
		SH_GAME_API void OnDestroy() override;
		SH_GAME_API void Awake() override;
		SH_GAME_API void Start() override;
		SH_GAME_API void OnEnable() override;
		SH_GAME_API void OnDisable() override;
		SH_GAME_API void FixedUpdate() override;
		SH_GAME_API void BeginUpdate() override;
		SH_GAME_API void Update() override;
		SH_GAME_API void LateUpdate() override;
		SH_GAME_API void OnCollisionEnter(Collision&& collision);
		SH_GAME_API void OnCollisionStay(Collision&& collision);
		SH_GAME_API void OnCollisionExit(Collision&& collision);
		SH_GAME_API void OnTriggerEnter(Collider& collider);
		SH_GAME_API void OnTriggerExit(Collider& collider);

		SH_GAME_API auto Serialize() const -> core::Json override;
		SH_GAME_API void Deserialize(const core::Json& json) override;
		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;

		SH_GAME_API void SetActive(bool b);

		/// @brief 자식에게 자신의 active 상태를 전파 (역직렬화용)
		SH_GAME_API void PropagateEnable();

		/// @brief 이미 만들어진 컴포넌트를 추가하는 함수. 
		/// 해당 컴포넌트의 gameObject가 이 gameObject와 다르면 추가 되지 않는다.
		/// @param component 컴포넌트 포인터
		SH_GAME_API void AddComponent(Component* component);
		/// @brief 컴포넌트를 우선 순위에 따라 정렬하는 함수. 프레임이 시작 될 때 정렬된다.
		SH_GAME_API void RequestSortComponents();

		SH_GAME_API auto Clone() const -> GameObject&;

		/// @brief FixedUpdate 다음에 호출 된다. OnCollision 함수들을 호출한다.
		SH_GAME_API void ProcessCollisionFunctions();

		SH_GAME_API auto GetComponents() const -> const std::vector<Component*>& { return components; }
		SH_GAME_API auto IsActive() const -> bool { return bParentEnable && bEnable; }
		SH_GAME_API auto IsStatic() const -> bool { return bStatic; }
	public:
		/// @brief 새 컴포넌트를 추가하는 함수
		/// @tparam T 컴포넌트 타입
		/// @return 추가된 컴포넌트 포인터
		template<typename T>
		auto AddComponent() -> std::enable_if_t<IsComponent<T>::value, T*>
		{
			components.push_back(core::SObject::Create<T>(*this));

			auto ptr = components.back();
			ptr->SetActive(true);
			ptr->SetName(components.back()->GetType().name);

			world.PublishEvent(events::ComponentEvent{ *ptr, events::ComponentEvent::Type::Added });

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
		template<typename T>
		auto GetComponentsInChildren(bool bIncludeDerived = false) const -> std::vector<T*>
		{
			std::vector<T*> result{};
			std::queue<Transform*> bfs{};
			bfs.push(transform);
			while (!bfs.empty())
			{
				Transform* const trans = bfs.front();
				bfs.pop();
				GameObject* obj = &trans->gameObject;
				for (Component* component : obj->components)
				{
					if (!core::IsValid(component))
						continue;
					if (!bIncludeDerived && component->GetType() == T::GetStaticType() ||
						bIncludeDerived && component->GetType().IsChildOf(T::GetStaticType()))
						result.push_back(static_cast<T*>(component));
				}
				for (Transform* child : trans->GetChildren())
					bfs.push(child);
			}
			return result;
		}
	private:
		void SortComponents();
	public:
		World& world;
		Transform* const transform;

		const bool& activeSelf;

		PROPERTY(hideInspector, core::PropertyOption::invisible)
		bool hideInspector = false;
		bool bNotSave = false;
		PROPERTY(bEditorOnly, core::PropertyOption::invisible)
		bool bEditorOnly = false;
	private:
		alignas(alignof(Transform)) std::array<uint8_t, sizeof(Transform)> transformBuffer;
		core::SVector<Component*> components;

		enum class ProcessingState
		{
			Enter,
			Stay,
			Exit
		};
		struct ProccessingTrigger
		{
			ProcessingState state;
			core::SObjWeakPtr<Collider, void> collider;

			ProccessingTrigger(ProcessingState state, Collider* colliderPtr) :
				state(state), collider(colliderPtr)
			{
			}
		};
		struct ProccessingCollision
		{
			ProcessingState state;
			Collision collision;

			ProccessingCollision(ProcessingState state, Collision&& collision) :
				state(state), collision(std::move(collision))
			{
			}
		};
		std::unordered_map<Collider*, std::size_t> processingColliderIdxs;
		std::vector<ProccessingTrigger> processingTriggers;
		std::vector<ProccessingCollision> processingCollisions;
		PROPERTY(bEnable)
		bool bEnable;
		bool bParentEnable = true;
		bool bRequestSortComponent = false;
		PROPERTY(bStatic)
		bool bStatic = false;
	};
}//namespace