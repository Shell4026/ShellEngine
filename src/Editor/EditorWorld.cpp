#include "Game/PCH.h"
#include "EditorWorld.h"
#include "EditorResource.h"
#include "EditorUI.h"

#include "Game/GameObject.h"
#include "Game/Component/MeshRenderer.h"
#include "Game/Component/EditorCamera.h"
#include "Game/Component/PickingCamera.h"
#include "Game/Component/PickingRenderer.h"
#include "Game/Component/EditorControl.h"
#include "Game/Component/LineRenderer.h"

namespace sh::editor
{
	SH_EDITOR_API EditorWorld::EditorWorld(render::Renderer& renderer, const game::ComponentModule& module, game::ImGUImpl& guiContext) :
		World(renderer, module), guiContext(guiContext)
	{
		game::GameObject* camObj = AddGameObject("EditorCamera");
		camObj->transform->SetPosition({ 2.f, 2.f, 2.f });
		camObj->hideInspector = true;
		editorCamera = camObj->AddComponent<game::EditorCamera>();
		editorCamera->SetUUID(core::UUID{ "61b7bc9f9fd2ca27dcbad8106745f62a" });
		this->SetMainCamera(editorCamera);

		auto PickingCamObj = AddGameObject("PickingCamera");
		PickingCamObj->transform->SetParent(camObj->transform);
		auto pickingCam = PickingCamObj->AddComponent<game::PickingCamera>();
		pickingCam->SetUUID(core::UUID{ "af9cac824334bcaccd86aad8a18e3cba" });
		pickingCam->SetFollowCamera(editorCamera);
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

	SH_EDITOR_API void EditorWorld::SetSelectedObject(core::SObject* obj)
	{
		if (core::IsValid(selected) && selected->GetType() == game::GameObject::GetStaticType())
		{
			if (core::IsValid(selected))
			{
				auto control = static_cast<game::GameObject*>(selected)->GetComponent<game::EditorControl>();
				if (core::IsValid(control))
					control->Destroy();
			}
		}
		selected = obj;
		if (core::IsValid(selected) && selected->GetType() == game::GameObject::GetStaticType())
		{
			if (static_cast<game::GameObject*>(selected)->GetComponent<game::EditorControl>() == nullptr)
			{
				auto control = static_cast<game::GameObject*>(selected)->AddComponent<game::EditorControl>();
				control->SetCamera(editorCamera);
				control->hideInspector = true;
			}
		}
	}
	SH_EDITOR_API auto EditorWorld::GetSelectedObject() const -> core::SObject*
	{
		return selected;
	}

	SH_EDITOR_API void EditorWorld::Start()
	{
		EditorResource::GetInstance()->LoadAllAssets(*this);

		editorUI = std::make_unique<editor::EditorUI>(*this, guiContext);
		this->GetMainCamera()->SetRenderTexture(editorUI->GetViewport().GetRenderTexture());

		auto grid = this->AddGameObject("Grid");
		grid->hideInspector = true;
		auto meshRenderer = grid->AddComponent<game::MeshRenderer>();
		meshRenderer->SetMesh(this->meshes.GetResource("GridMesh"));
		meshRenderer->SetMaterial(this->materials.GetResource("GridMaterial"));

		auto axis = AddGameObject("_Axis");
		axis->transform->SetPosition(0.f, 0.01f, 0.f);
		axis->transform->SetParent(grid->transform);
		auto line = axis->AddComponent<game::LineRenderer>();
		line->SetEnd({ 10.f, 0.f, 0.f });
		line->SetColor({ 1.f, 0.f, 0.f, 1.f });
		line = axis->AddComponent<game::LineRenderer>();
		line->SetEnd({ 0.f, 10.f, 0.f });
		line->SetColor({ 0.f, 1.f, 0.f, 1.f });
		line = axis->AddComponent<game::LineRenderer>();
		line->SetEnd({ 0.f, 0.f, 10.f });
		line->SetColor({ 0.f, 0.f, 1.f, 1.f });

		Super::Start();
	}

	SH_EDITOR_API void EditorWorld::Update(float deltaTime)
	{
		guiContext.Begin();

		editorUI->Update();
		editorUI->Render();

		guiContext.End();
		Super::Update(deltaTime);
	}

	SH_EDITOR_API auto EditorWorld::Serialize() const -> core::Json
	{
		return Super::Serialize();
	}
	SH_EDITOR_API void EditorWorld::Deserialize(const core::Json& json)
	{
		Super::Deserialize(json);
	}
}