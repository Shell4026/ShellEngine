#pragma once
#include "Export.h"

#include "Core/Observer.hpp"
#include "Core/EventSubscriber.h"

#include "Game/World.h"
#include "Game/WorldEvents.hpp"

namespace sh::game
{
	class ImGUImpl;
	class EditorCamera;
	class PickingCamera;
}
namespace sh::render
{
	class TransparentPipeline;
}
namespace sh::editor
{
	class Project;
	class EditorUI;
	class EditorPickingPass;
	class EditorOutlinePass;
	class EditorPostOutlinePass;
	/// @brief 에디터용 월드 클래스
	class EditorWorld : public game::World
	{
		SCLASS(EditorWorld)
	private:
		Project& project;

		core::SVector<core::SObject*> selectedObjs;

		PROPERTY(viewportTexture, core::PropertyOption::noSave)
		render::RenderTexture* viewportTexture = nullptr;
		PROPERTY(editorCamera)
		game::EditorCamera* editorCamera = nullptr;
		game::PickingCamera* pickingCamera = nullptr;
		game::GameObject* grid = nullptr;
		game::GameObject* axis = nullptr;

		EditorUI* editorUI = nullptr;

		EditorPickingPass* pickingPass = nullptr;
		EditorOutlinePass* outlinePass = nullptr;
		EditorPostOutlinePass* postOutlinePass = nullptr;
		render::TransparentPipeline* transParentPass = nullptr;

		core::EventSubscriber<game::events::ComponentEvent> componentSubscriber;
	private:
		void AddEditorControlsToSelected(core::SObject& obj);
		void RemoveEditorControls(core::SObject& obj);
		void AddOrDestroyOutlineComponent(game::GameObject& obj, bool bAdd);
	public:
		SH_EDITOR_API EditorWorld(Project& project);
		SH_EDITOR_API ~EditorWorld();

		SH_EDITOR_API void OnDestroy() override;

		SH_EDITOR_API void Clear() override;

		SH_EDITOR_API void SetRenderPass() override;
		SH_EDITOR_API void InitResource() override;

		SH_EDITOR_API void AddSelectedObject(core::SObject* obj);
		SH_EDITOR_API auto GetSelectedObjects() const -> const core::SVector<SObject*>&;
		SH_EDITOR_API void ClearSelectedObjects();

		SH_EDITOR_API auto IsSelected(core::SObject* obj) const -> bool;

		SH_EDITOR_API auto AddGameObject(std::string_view name) -> game::GameObject* override;
		SH_EDITOR_API auto DuplicateGameObject(const game::GameObject& obj) -> game::GameObject& override;
		SH_EDITOR_API auto GetEditorUI() const -> EditorUI&;
		SH_EDITOR_API auto GetViewportTexture() const-> render::RenderTexture&;

		SH_EDITOR_API void Start() override;

		SH_EDITOR_API auto Serialize() const -> core::Json override;
		SH_EDITOR_API void Deserialize(const core::Json& json) override;
	};
}