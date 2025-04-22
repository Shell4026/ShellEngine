﻿#include "Component/UI.h"
#include "GameObject.h"
#include "ImGUImpl.h"

namespace sh::game
{
	UI::UI(GameObject& owner) :
		Component(owner)
	{
	}

	SH_GAME_API void UI::Update()
	{
		gameObject.world.GetUiContext().SyncDirty();
	}
}//namespace