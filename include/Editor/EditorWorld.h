#pragma once

#include "Export.h"

#include "Game/World.h"

namespace sh::editor
{
	/// @brief 에디터용 월드 클래스
	class EditorWorld : public game::World
	{
		SCLASS(EditorWorld)
	private:
		PROPERTY(selected)
		game::GameObject* selected = nullptr;
	public:
		SH_EDITOR_API EditorWorld(render::Renderer&, const game::ComponentModule& module);
		SH_EDITOR_API ~EditorWorld();

		SH_EDITOR_API void SetSelectedObject(game::GameObject* gameObject);
		SH_EDITOR_API auto GetSelectedObject() const -> game::GameObject*;
	};
}