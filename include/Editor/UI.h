#pragma once

#include "Export.h"

#include "Core/ISyncable.h"

#include "Game/ImGUImpl.h"

namespace sh::editor
{
	class UI
	{
	protected:
		game::ImGUImpl& imgui;
	public:
		SH_EDITOR_API UI(game::ImGUImpl& imgui);

		SH_EDITOR_API virtual void Update() = 0;
		SH_EDITOR_API virtual void Render() = 0;
	};
}