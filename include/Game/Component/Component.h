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
		_ComponentBuilder_##className()\
		{\
			sh::game::ComponentModule::GetInstance()->RegisterComponent<className>(#className, std::string{__VA_ARGS__});\
		}\
		static auto GetStatic() -> _ComponentBuilder_##className*\
		{\
			static _ComponentBuilder_##className builder{};\
			return &builder;\
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
	private:
		bool bEnable;
		bool bInit;
	public:
		GameObject& gameObject;
		World& world;
		const bool& active;
#if SH_EDITOR
		PROPERTY(hideInspector, core::PropertyOption::invisible)
		bool hideInspector = false;
#endif
	public:
		SH_GAME_API Component(GameObject& object);
		SH_GAME_API virtual ~Component() = default;
		SH_GAME_API Component(const Component& other);
		SH_GAME_API Component(Component&& other) noexcept;

		SH_GAME_API void SetActive(bool b);

		SH_GAME_API void Awake() override;
		SH_GAME_API void Start() override;
		SH_GAME_API void OnEnable() override;
		SH_GAME_API void FixedUpdate() override;
		SH_GAME_API void BeginUpdate() override;
		SH_GAME_API void Update() override;
		SH_GAME_API void LateUpdate() override;
		SH_GAME_API void OnDestroy() override;

		SH_GAME_API auto Serialize() const -> core::Json override;
		SH_GAME_API void Deserialize(const core::Json& json) override;
	};

	template<typename T>
	struct IsComponent : std::bool_constant<std::is_base_of<Component, T>::value>
	{
	};
}