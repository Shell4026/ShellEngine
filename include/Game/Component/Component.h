#pragma once
#include "Game/Export.h"
#include "Game/IObject.h"
#include "Game/ComponentModule.h"

#include "Core/Util.h"
#include "Core/SObject.h"

#include <type_traits>

/// 컴포넌트 임을 명시하고 자동으로 컴포넌트 모듈에 해당 컴포넌트를 등록하는 매크로.
/// className 클래스 이름
/// ... 그룹 이름
#define COMPONENT(className, ...)\
	SCLASS(className)\
	struct _ComponentBuilder_##className\
	{\
		~_ComponentBuilder_##className()\
		{\
			auto componentModule = sh::game::ComponentModule::GetInstance();\
			componentModule->DestroyComponent(#className, std::string{__VA_ARGS__});\
			SH_INFO_FORMAT("Component({}) was unloaded.", #className);\
		}\
		static auto GetStatic() -> _ComponentBuilder_##className*\
		{\
			auto componentModule = sh::game::ComponentModule::GetInstance();\
			if (componentModule->GetComponent(#className) == nullptr) \
			{\
				componentModule->RegisterComponent<className>(#className, std::string{__VA_ARGS__});\
				static _ComponentBuilder_##className builder{};\
				return &builder;\
			}\
			return nullptr;\
		}\
	}; \
	inline static _ComponentBuilder_##className* _componentBuilder = _ComponentBuilder_##className::GetStatic();

namespace sh::game
{
	class World;
	class GameObject;

	class Component : public sh::core::SObject, public IObject
	{
		SCLASS(Component)
		friend class GameObject;
	public:
		SH_GAME_API Component(GameObject& object);
		SH_GAME_API virtual ~Component() = default;
		SH_GAME_API Component(const Component& other);
		SH_GAME_API Component(Component&& other) noexcept;

		SH_GAME_API auto operator=(const Component& other) -> Component&;

		SH_GAME_API auto IsActive() const -> bool;
		SH_GAME_API virtual void SetActive(bool b);

		SH_GAME_API auto IsInit() const -> bool;
		SH_GAME_API auto IsStart() const -> bool;

		SH_GAME_API void OnDestroy() override;

		SH_GAME_API void Awake() override {}
		SH_GAME_API void Start() override {}
		SH_GAME_API void OnEnable() override {}
		SH_GAME_API void FixedUpdate() override {}
		SH_GAME_API void BeginUpdate() override {}
		SH_GAME_API void Update() override {}
		SH_GAME_API void LateUpdate() override {}
		SH_GAME_API void OnCollisionEnter(Collider& collider) override {}
		SH_GAME_API void OnCollisionStay(Collider& collider) override {}
		SH_GAME_API void OnCollisionExit(Collider& collider) override {}

		/// @brief 우선 순위가 높을수록 다른 컴포넌트보다 우선 실행 된다.
		/// @param priority 우선 순위
		SH_GAME_API void SetPriority(int priority);
		SH_GAME_API auto GetPriority() const -> int;

		SH_GAME_API auto Serialize() const -> core::Json override;
		SH_GAME_API void Deserialize(const core::Json& json) override;

		SH_GAME_API static void SetEditor(bool bEditor);
		/// @brief 현재 에디터에서 실행중인지 반환
		/// @return 에디터라면 True, 아니라면 False
		SH_GAME_API static auto IsEditor() -> bool;
	public:
		GameObject& gameObject;
		World& world;

		PROPERTY(hideInspector, core::PropertyOption::invisible)
		bool hideInspector = false;
		bool canPlayInEditor = false;
	private:
		PROPERTY(priority, core::PropertyOption::invisible)
		int priority = 0;

		bool bEnable;
		bool bInit;
		bool bStart = false;

		static bool bEditor;
	};

	template<typename T>
	struct IsComponent : std::bool_constant<std::is_base_of<Component, T>::value>
	{
	};
}//namespace