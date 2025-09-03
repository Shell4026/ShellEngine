#include "EditorWorld.h"
#include "EditorResource.h"
#include "EditorPickingPass.h"
#include "EditorOutlinePass.h"
#include "EditorPostOutlinePass.h"
#include "UI/Project.h"
#include "AssetDatabase.h"

#include "Component/EditorUI.h"
#include "Component/OutlineComponent.h"

#include "Core/Name.h"

#include "Render/Mesh/Grid.h"
#include "Render/Model.h"
#include "Render/TransparentPipeline.h"

#include "Editor/Component/EditorControl.h"

#include "Game/GameObject.h"
#include "Game/Component/MeshRenderer.h"
#include "Game/Component/EditorCamera.h"
#include "Game/Component/PickingCamera.h"
#include "Game/Component/PickingRenderer.h"
#include "Game/Component/LineRenderer.h"
#include "Game/WorldEvents.hpp"

namespace sh::editor
{
	void EditorWorld::AddEditorControlsToSelected(core::SObject& obj)
	{
		if (obj.GetType() == game::GameObject::GetStaticType())
		{
			game::GameObject* selectedObj = static_cast<game::GameObject*>(&obj);

			EditorControl* control = selectedObj->GetComponent<EditorControl>();
			if (control == nullptr)
			{
				control = selectedObj->AddComponent<EditorControl>();
				control->SetCamera(editorCamera);
				control->hideInspector = true;
				if (IsPlaying())
					control->SetActive(false);
			}
			AddOrDestroyOutlineComponent(*selectedObj, true);
		}
	}
	void EditorWorld::RemoveEditorControls(core::SObject& obj)
	{
		if (obj.GetType() == game::GameObject::GetStaticType())
		{
			game::GameObject* selectedObj = static_cast<game::GameObject*>(&obj);
			auto control = selectedObj->GetComponent<EditorControl>();
			if (core::IsValid(control))
				control->Destroy();
			AddOrDestroyOutlineComponent(*selectedObj, false);
		}
	}
	void EditorWorld::AddOrDestroyOutlineComponent(game::GameObject& obj, bool bAdd)
	{
		for (auto child : obj.transform->GetChildren())
			AddOrDestroyOutlineComponent(child->gameObject, bAdd);

		auto outline = obj.GetComponent<editor::OutlineComponent>();
		if (!core::IsValid(outline))
		{
			if (bAdd)
			{
				auto outlineComponent = obj.AddComponent<editor::OutlineComponent>();
				outlineComponent->hideInspector = true;
			}
		}
		else
		{
			if (!bAdd)
			{
				outline->Destroy();
			}
		}
	}

	SH_EDITOR_API EditorWorld::EditorWorld(Project& project) :
		World(project.renderer, project.gui),
		project(project)
	{
		componentSubscriber.SetCallback(
			[&](const game::events::ComponentEvent& event)
			{
				game::Component& component = event.component;
				game::GameObject& obj = component.gameObject;
				
				if (&obj == grid || &obj == axis)
					return;
				if (component.GetType() == game::MeshRenderer::GetStaticType())
				{
					auto pickingRenderer = obj.AddComponent<game::PickingRenderer>();
					pickingRenderer->hideInspector = true;
					pickingRenderer->SetCamera(*pickingCamera);
					pickingRenderer->SetMeshRenderer(static_cast<game::MeshRenderer&>(component));
				}
			}
		);
		eventBus.Subscribe(componentSubscriber);

		ImGui::SetCurrentContext(project.gui.GetContext());
	}

	SH_EDITOR_API EditorWorld::~EditorWorld()
	{
		AssetDatabase::GetInstance()->SaveAllAssets();
	}

	SH_EDITOR_API void EditorWorld::OnDestroy()
	{

		Super::OnDestroy();
	}

	SH_EDITOR_API void EditorWorld::Clean()
	{
		Super::Clean();
		editorUI = nullptr;
		selectedObjs.clear();
	}

	SH_EDITOR_API void EditorWorld::SetRenderPass()
	{
		pickingPass = renderer.AddRenderPipeline<EditorPickingPass>();
		outlinePass = renderer.AddRenderPipeline<EditorOutlinePass>();
		renderer.AddRenderPipeline<render::RenderPipeline>();
		transParentPass = renderer.AddRenderPipeline<render::TransparentPipeline>();
		postOutlinePass = renderer.AddRenderPipeline<EditorPostOutlinePass>();
	}
	SH_EDITOR_API void EditorWorld::InitResource()
	{
		Super::InitResource();

		render::RenderTexture* viewportTexture = core::SObject::Create<render::RenderTexture>(render::Texture::TextureFormat::SRGBA32);
		viewportTexture->SetUUID(core::UUID{ "180635b4e4d1a98ebb0064ab47dc452a" });
		viewportTexture->Build(*renderer.GetContext());
		textures.AddResource("Viewport", viewportTexture);

		game::GameObject* camObj = AddGameObject("EditorCamera");
		camObj->SetUUID(core::UUID{ "e071c2f9898d000823c56389418e8147" });
		camObj->transform->SetPosition({ 2.f, 2.f, 2.f });
		camObj->hideInspector = true;
		camObj->bNotSave = true;
		editorCamera = camObj->AddComponent<game::EditorCamera>();
		editorCamera->SetUUID(core::UUID{ "61b7bc9f9fd2ca27dcbad8106745f62a" });
		editorCamera->SetRenderTexture(viewportTexture);
		editorCamera->GetNative().SetActive(true);

		auto PickingCamObj = AddGameObject("PickingCamera");
		PickingCamObj->bNotSave = true;
		PickingCamObj->SetUUID(core::UUID{ "94702dba2122b976d2941638731507fa" });
		PickingCamObj->transform->SetParent(camObj->transform);
		PickingCamObj->transform->SetPosition({ 0.f, 0.f, 0.f });
		pickingCamera = PickingCamObj->AddComponent<game::PickingCamera>();
		pickingCamera->SetUUID(core::UUID{ "af9cac824334bcaccd86aad8a18e3cba" });
		pickingCamera->SetFollowCamera(editorCamera);
		pickingCamera->GetNative().SetActive(true);

		pickingPass->SetCamera(*pickingCamera);
		outlinePass->SetCamera(*editorCamera);

		auto outlineTexture = EditorResource::GetInstance()->GetTexture("OutlineTexture");
		assert(outlineTexture);
		outlinePass->SetOutTexture(static_cast<render::RenderTexture&>(*outlineTexture));

		postOutlinePass->SetCamera(*editorCamera);
		auto outlinePostMat = EditorResource::GetInstance()->GetMaterial("OutlinePostMaterial");
		assert(outlinePostMat);
		postOutlinePass->SetOutlineMaterial(*outlinePostMat);

		renderer.GetRenderPipeline(core::Name{ "Forward" })->IgnoreCamera(pickingCamera->GetNative());
		transParentPass->IgnoreCamera(pickingCamera->GetNative());
		// 렌더 테스트용 객체
		//auto prop = core::SObject::Create<render::MaterialPropertyBlock>();
		//prop->SetProperty("offset", glm::vec2{ 0.5f, 0.f });

		//game::GameObject* tri = AddGameObject("Triangle");
		//tri->transform->SetScale(0.5f, 1.0f, 1.f);
		//auto m = tri->AddComponent<game::MeshRenderer>();
		//m->SetMaterial(triMat);
		//m->SetMesh(plane);
		//m->SetMaterialPropertyBlock(prop);

		game::GameObject* uiObj = AddGameObject("EditorUI");
		uiObj->hideInspector = true;
		uiObj->bNotSave = true;
		editorUI = uiObj->AddComponent<EditorUI>();
		editorUI->SetProject(project);

		grid = AddGameObject("Grid"); // Grid와 Axis는 피킹 렌더러가 추가 되면 안 된다.
		grid->hideInspector = true;
		grid->bNotSave = true;
		auto meshRenderer = grid->AddComponent<game::MeshRenderer>();
		auto gridMesh = core::SObject::Create<render::Grid>();
		gridMesh->Build(*renderer.GetContext());
		meshRenderer->SetMesh(gridMesh);
		meshRenderer->SetMaterial(EditorResource::GetInstance()->GetMaterial("GridMaterial"));

		axis = AddGameObject("Axis");
		axis->hideInspector = true;
		axis->bNotSave = true;
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
	}

	SH_EDITOR_API void EditorWorld::AddSelectedObject(core::SObject* obj)
	{
		if (IsSelected(obj) || !core::IsValid(obj))
			return;

		selectedObjs.push_back(obj);
		
		AddEditorControlsToSelected(*obj);
	}

	SH_EDITOR_API auto EditorWorld::GetSelectedObjects() const -> const core::SVector<SObject*>&
	{
		return selectedObjs;
	}

	SH_EDITOR_API void EditorWorld::ClearSelectedObjects()
	{
		for (auto obj : selectedObjs)
		{
			if (!core::IsValid(obj))
				continue;
			
			RemoveEditorControls(*obj);
		}
		selectedObjs.clear();
	}

	SH_EDITOR_API auto EditorWorld::IsSelected(core::SObject* obj) const -> bool
	{
		if (obj == nullptr)
			return false;

		for (auto selected : selectedObjs)
		{
			if (selected == obj)
				return true;
		}
		return false;
	}

	SH_EDITOR_API auto EditorWorld::AddGameObject(std::string_view name) -> game::GameObject*
	{
		auto obj = Super::AddGameObject(name);
		return obj;
	}

	SH_EDITOR_API auto EditorWorld::DuplicateGameObject(const game::GameObject& obj) -> game::GameObject&
	{
		game::GameObject& dup = Super::DuplicateGameObject(obj);

		std::queue<game::GameObject*> bfs{};
		bfs.push(&dup);
		while (!bfs.empty())
		{
			auto obj = bfs.front();
			bfs.pop();
			if (!core::IsValid(obj))
				continue;
			auto meshRenderer = obj->GetComponent<game::MeshRenderer>();
			if (meshRenderer != nullptr)
			{
				auto pickingRenderer = obj->AddComponent<game::PickingRenderer>();
				pickingRenderer->hideInspector = true;
				pickingRenderer->SetCamera(*pickingCamera);
				pickingRenderer->SetMeshRenderer(*meshRenderer);
			}
			for (auto child : obj->transform->GetChildren())
				bfs.push(&child->gameObject);
		}

		return dup;
	}

	SH_EDITOR_API auto EditorWorld::GetEditorUI() const -> EditorUI&
	{
		return *editorUI;
	}

	SH_EDITOR_API void EditorWorld::Start()
	{
		//auto tri = this->AddGameObject("Triangle");
		//auto meshRenderer = tri->AddComponent<game::MeshRenderer>();
		//meshRenderer->SetMesh(this->meshes.GetResource("PlaneMesh"));
		//meshRenderer->SetMaterial(this->materials.GetResource("TriangleMaterial"));
		//auto propBlock = core::SObject::Create<render::MaterialPropertyBlock>();
		//propBlock->SetProperty("offset", glm::vec2{ 0.0f, 0.0f });
		//meshRenderer->SetMaterialPropertyBlock(propBlock);

		Super::Start();
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