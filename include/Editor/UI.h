#pragma once

#include "Export.h"

#include "Core/ISyncable.h"

#include "Game/ImGUI.h"

namespace sh::editor
{
	class UI
	{
	protected:
		game::ImGUI& imgui;
	public:
		SH_EDITOR_API UI(game::ImGUI& imgui);

		SH_EDITOR_API virtual void Update() = 0;
	};
}