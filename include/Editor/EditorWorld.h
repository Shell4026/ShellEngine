#pragma once

#include "Export.h"

#include "Game/World.h"

namespace sh::game
{
	class ImGUImpl;
	class EditorCamera;
}
namespace sh::editor
{
	class EditorUI;
	/// @brief 에디터용 월드 클래스
	class EditorWorld : public game::World
	{
		SCLASS(EditorWorld)
	private:
		PROPERTY(selected)
		game::GameObject* selected = nullptr;
		PROPERTY(editorCamera)
		game::EditorCamera* editorCamera = nullptr;

		game::ImGUImpl& guiContext;

		std::unique_ptr<EditorUI> editorUI;
	public:
		SH_EDITOR_API EditorWorld(render::Renderer&, const game::ComponentModule& module, game::ImGUImpl& guiContext);
		SH_EDITOR_API ~EditorWorld();

		SH_EDITOR_API void Clean() override;

		SH_EDITOR_API void SetSelectedObject(game::GameObject* gameObject);
		SH_EDITOR_API auto GetSelectedObject() const -> game::GameObject*;

		SH_EDITOR_API void Start() override;
		SH_EDITOR_API void Update(float deltaTime) override;
	};
}