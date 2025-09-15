#pragma once

#include "Export.h"

namespace sh::game
{
	class Collider;
	/// @brief 게임 상태 업데이트 함수들을 정의해둔 인터페이스
	class IObject
	{
	public:
		SH_GAME_API ~IObject() = default;
		SH_GAME_API virtual void Awake() = 0;
		SH_GAME_API virtual void Start() = 0;
		SH_GAME_API virtual void OnEnable() = 0;
		SH_GAME_API virtual void OnDisable() = 0;
		SH_GAME_API virtual void BeginUpdate() = 0;
		SH_GAME_API virtual void Update() = 0;
		SH_GAME_API virtual void LateUpdate() = 0;
		SH_GAME_API virtual void FixedUpdate() = 0;
		SH_GAME_API virtual void OnCollisionEnter(Collider& collider) = 0;
		SH_GAME_API virtual void OnCollisionStay(Collider& collider) = 0;
		SH_GAME_API virtual void OnCollisionExit(Collider& collider) = 0;
	};
}