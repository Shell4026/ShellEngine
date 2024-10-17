#pragma once

#include "Game/Export.h"
#include "Game/Object.h"
#include "Game/ComponentModule.h"

#include "Core/Util.h"

#include <type_traits>

#define COMPONENT(className, folderName)\
	SCLASS(className)\
	struct _ComponentBuilder_##className\
	{\
		_ComponentBuilder_##className()\
		{\
			sh::game::ComponentModule::GetInstance()->RegisterComponent<className>(#className, std::string{folderName});\
		}\
	};\
	inline static _ComponentBuilder_##className* _componentBuilder = &static _ComponentBuilder_##className{};

namespace sh::game
{
	class GameObject;

	class Component : public IObject
	{
		SCLASS(Component)
	private:
		bool bEnable;
		bool bInit;
	public:
		GameObject* gameObject;
		const bool& active;
	protected:
		auto GetComponentModule() const -> ComponentModule*;
	public:
		SH_GAME_API Component();
		SH_GAME_API virtual ~Component() = default;
		SH_GAME_API Component(const Component& other);
		SH_GAME_API Component(Component&& other) noexcept;

		SH_GAME_API void SetOwner(GameObject& object);
		SH_GAME_API void SetActive(bool b);

		SH_GAME_API void Awake() override;
		SH_GAME_API void Start() override;
		SH_GAME_API void OnEnable() override;
		SH_GAME_API void BeginUpdate() override;
		SH_GAME_API void Update() override;
		SH_GAME_API void LateUpdate() override;
		SH_GAME_API void OnDestroy() override;
	};

	template<typename T>
	struct IsComponent : std::bool_constant<std::is_base_of<Component, T>::value>
	{
	};
}