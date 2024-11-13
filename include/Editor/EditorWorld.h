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
		core::SObject* selected = nullptr;
		PROPERTY(editorCamera)
		game::EditorCamera* editorCamera = nullptr;

		game::ImGUImpl& guiContext;

		std::unique_ptr<EditorUI> editorUI;
	public:
		SH_EDITOR_API EditorWorld(render::Renderer&, const game::ComponentModule& module, game::ImGUImpl& guiContext);
		SH_EDITOR_API ~EditorWorld();

		SH_EDITOR_API void Clean() override;

		SH_EDITOR_API void SetSelectedObject(core::SObject* obj);
		SH_EDITOR_API auto GetSelectedObject() const -> core::SObject*;

		SH_EDITOR_API void Start() override;
		SH_EDITOR_API void Update(float deltaTime) override;

		SH_EDITOR_API auto Serialize() const -> core::Json override;
		SH_EDITOR_API void Deserialize(const core::Json& json) override;
	};
}