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
		core::SVector<core::SObject*> selectedObjs;

		PROPERTY(editorCamera)
		game::EditorCamera* editorCamera = nullptr;
		game::PickingCamera* pickingCamera = nullptr;
		game::GameObject* grid = nullptr;
		game::GameObject* axis = nullptr;

		EditorUI* editorUI = nullptr;

		EditorPickingPass* pickingPass = nullptr;
		EditorOutlinePass* outlinePass = nullptr;
		EditorPostOutlinePass* postOutlinePass = nullptr;

		core::Observer<false, game::Component*>::Listener onComponentAddListener;
	public:
		mutable core::Observer<false, game::GameObject*, game::Component*> onComponentAdd;
	private:
		void AddEditorControlsToSelected(core::SObject& obj);
		void RemoveEditorControls(core::SObject& obj);
		void AddOrDestroyOutlineComponent(game::GameObject& obj, bool bAdd);
	public:
		SH_EDITOR_API EditorWorld(render::Renderer&, const game::ComponentModule& module, game::ImGUImpl& guiContext);
		SH_EDITOR_API ~EditorWorld();

		SH_EDITOR_API void Clean() override;

		SH_EDITOR_API void InitResource() override;

		SH_EDITOR_API void AddSelectedObject(core::SObject* obj);
		SH_EDITOR_API auto GetSelectedObjects() const -> const core::SVector<SObject*>&;
		SH_EDITOR_API void ClearSelectedObjects();

		SH_EDITOR_API auto IsSelected(core::SObject* obj) const -> bool;

		SH_EDITOR_API auto AddGameObject(std::string_view name) -> game::GameObject* override;
		SH_EDITOR_API auto DuplicateGameObject(const game::GameObject& obj) -> game::GameObject& override;
		SH_EDITOR_API auto GetEditorUI() const -> EditorUI&;

		SH_EDITOR_API void Start() override;

		SH_EDITOR_API auto Serialize() const -> core::Json override;
		SH_EDITOR_API void Deserialize(const core::Json& json) override;
	};
}