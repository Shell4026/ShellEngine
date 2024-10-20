#include "Game/PCH.h"
#include "EditorWorld.h"
#include "EditorResource.h"
#include "EditorUI.h"

#include "Game/GameObject.h"
#include "Game/Component/MeshRenderer.h"
#include "Game/Component/EditorCamera.h"
#include "Game/Component/PickingCamera.h"
#include "Game/Component/PickingRenderer.h"

namespace sh::editor
{
	SH_EDITOR_API EditorWorld::EditorWorld(render::Renderer& renderer, const game::ComponentModule& module, game::ImGUImpl& guiContext) :
		World(renderer, module), guiContext(guiContext)
	{
		game::GameObject* camObj = AddGameObject("EditorCamera");
		camObj->transform->SetPosition({ 2.f, 2.f, 2.f });
		auto cam = camObj->AddComponent<game::EditorCamera>();
		this->SetMainCamera(cam);

		auto PickingCamObj = this->AddGameObject("PickingCamera");
		PickingCamObj->transform->SetParent(camObj->transform);
		auto pickingCam = PickingCamObj->AddComponent<game::PickingCamera>();
		pickingCam->SetFollowCamera(cam);
	}

	SH_EDITOR_API EditorWorld::~EditorWorld()
	{
	}

	SH_EDITOR_API void EditorWorld::Clean()
	{
		Super::Clean();
		editorUI.reset();
		selected = nullptr;
	}

	SH_EDITOR_API void EditorWorld::SetSelectedObject(game::GameObject* gameObject)
	{
		selected = gameObject;
	}
	SH_EDITOR_API auto EditorWorld::GetSelectedObject() const -> game::GameObject*
	{
		return selected;
	}

	SH_EDITOR_API void EditorWorld::Start()
	{
		EditorResource::GetInstance()->LoadAllAssets(*this);

		editorUI = std::make_unique<editor::EditorUI>(*this, guiContext);
		this->GetMainCamera()->SetRenderTexture(editorUI->GetViewport().GetRenderTexture());

		auto grid = this->AddGameObject("Grid");
		auto meshRenderer = grid->AddComponent<game::MeshRenderer>();
		meshRenderer->SetMesh(this->meshes.GetResource("GridMesh"));
		meshRenderer->SetMaterial(this->materials.GetResource("GridMaterial"));

		Super::Start();
	}

	SH_EDITOR_API void EditorWorld::Update(float deltaTime)
	{
		Super::Update(deltaTime);
		guiContext.Begin();

		editorUI->Update();
		editorUI->Render();

		guiContext.End();
	}
}