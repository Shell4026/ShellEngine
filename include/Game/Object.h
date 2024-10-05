#pragma once

#include "Export.h"
#include "Core/SObject.h"

namespace sh::game
{
	class IObject : public sh::core::SObject
	{
	public:
		SH_GAME_API virtual void Awake() = 0;
		SH_GAME_API virtual void Start() = 0;
		SH_GAME_API virtual void OnEnable() = 0;
		SH_GAME_API virtual void BeginUpdate() = 0;
		SH_GAME_API virtual void Update() = 0;
		SH_GAME_API virtual void LateUpdate() = 0;
	};
}