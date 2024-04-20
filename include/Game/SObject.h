#pragma once

#include "Export.h"

#include "Core/Reflaction.hpp"

namespace sh::game
{
	class ISObject
	{
		SCLASS(ISObject)
	public:
		SH_GAME_API virtual void Awake() = 0;
		SH_GAME_API virtual void Start() = 0;
		SH_GAME_API virtual void OnEnable() = 0;
		SH_GAME_API virtual void Update() = 0;
		SH_GAME_API virtual void LateUpdate() = 0;
	};
}