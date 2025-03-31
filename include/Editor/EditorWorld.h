#pragma once
#include "Export.h"

#include "Core/Observer.hpp"

#include "Game/World.h"

namespace sh::game
{
	class ImGUImpl;
	class EditorCamera;
	class PickingCamera;
}
namespace sh::editor
{
	class EditorUI;
	class EditorPickingPass;
	class EditorOutlinePass;
	class EditorPostOutlinePass;
	/// @brief 에디터용 월드 클래스
	class EditorWorld : public game::World
	{
		SCLASS(EditorWorld)
	private:
		PROPERTY(selected)
		PROPERTY(editorCamera)

		core::SObject* selected = nullptr;
		
		game::EditorCamera* editorCamera = nullptr;
		game::PickingCamera* pickingCamera = nullptr;
		game::GameObject* grid = nullptr;
		game::GameObject* axis = nullptr;

		game::ImGUImpl& guiContext;

		std::unique_ptr<EditorUI> editorUI;

		EditorPickingPass* pickingPass = nullptr;
		EditorOutlinePass* outlinePass = nullptr;
		EditorPostOutlinePass* postOutlinePass = nullptr;

		core::Observer<false, game::Component*>::Listener onComponentAddListener;
	public:
		mutable core::Observer<false, game::GameObject*, game::Component*> onComponentAdd;
	public:
		SH_EDITOR_API EditorWorld(render::Renderer&, const game::ComponentModule& module, game::ImGUImpl& guiContext);
		SH_EDITOR_API ~EditorWorld();

		SH_EDITOR_API void Clean() override;

		SH_EDITOR_API void InitResource() override;

		SH_EDITOR_API void SetSelectedObject(core::SObject* obj);
		SH_EDITOR_API auto GetSelectedObject() const -> core::SObject*;

		SH_EDITOR_API  auto AddGameObject(std::string_view name) -> game::GameObject* override;

		SH_EDITOR_API void Start() override;
		SH_EDITOR_API void Update(float deltaTime) override;

		SH_EDITOR_API auto Serialize() const -> core::Json override;
		SH_EDITOR_API void Deserialize(const core::Json& json) override;
	};
}