#pragma once

#include "Game/Export.h"

#include "Game/Object.h"

#include "Core/Util.h"

#include <type_traits>

namespace sh::game
{
	class GameObject;

	class Component : public IObject
	{
		SCLASS(Component)
	private:
		bool bEnable;
		bool bInit : 1;
	public:
		GameObject& gameObject;
		const bool& active;
	public:
		SH_GAME_API Component(GameObject& owner);
		SH_GAME_API virtual ~Component() = default;
		SH_GAME_API Component(const Component& other);
		SH_GAME_API Component(Component&& other) noexcept;

		SH_GAME_API void SetActive(bool b);

		SH_GAME_API void Awake() override;
		SH_GAME_API void Start() override;
		SH_GAME_API void OnEnable() override;
		SH_GAME_API void Update() override;
		SH_GAME_API void LateUpdate() override;
	};

	template<typename T>
	struct IsComponent : std::bool_constant<std::is_base_of<Component, T>::value>
	{
	};
}