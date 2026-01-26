#include "EditorWorld.h"
#include "EditorResource.h"
#include "EditorRenderer.h"
#include "UI/Project.h"
#include "AssetDatabase.h"

#include "Component/EditorUI.h"
#include "Component/OutlineComponent.h"

#include "Core/Name.h"

#include "Render/Mesh/Grid.h"
#include "Render/Model.h"

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

	SH_EDITOR_API void EditorWorld::Clear()
	{
		Super::Clear();
		editorUI = nullptr;
		selectedObjs.clear();
		if (viewportTexture != nullptr)
		{
			viewportTexture->Destroy();
			viewportTexture = nullptr;
		}
		if (editorCamera != nullptr)
		{
			editorCamera->Destroy();
			editorCamera = nullptr;
		}
	}

	SH_EDITOR_API void EditorWorld::SetupRenderer()
	{
		customRenderer = std::make_unique<EditorRenderer>(*renderer.GetContext(), GetUiContext());
		renderer.SetScriptableRenderer(*customRenderer);
		auto& editorRenderer = static_cast<EditorRenderer&>(*customRenderer);

		game::GameObject* uicamObj = AddGameObject("UICamera");
		uicamObj->transform->SetPosition({ 2.f, 2.f, 2.f });
		uicamObj->hideInspector = true;
		uicamObj->bNotSave = true;
		game::Camera* cam = uicamObj->AddComponent<game::Camera>();
		cam->SetDepth(1000);

		editorRenderer.SetImGUICamera(cam->GetNative());
	}
	SH_EDITOR_API void EditorWorld::InitResource()
	{
		Super::InitResource();
		auto* editorRenderer = static_cast<EditorRenderer*>(customRenderer.get());

		// editorRenderer가 없으면 addtive로 추가된 월드임
		if (editorRenderer != nullptr)
		{
			auto viewportPtr = static_cast<render::RenderTexture*>(core::SObjectManager::GetInstance()->GetSObject(core::UUID{ "180635b4e4d1a98ebb0064ab47dc452a" }));
			if (!core::IsValid(viewportPtr))
			{
				if (viewportPtr != nullptr && viewportPtr->IsPendingKill())
					viewportPtr->SetUUID(core::UUID::Generate());

				render::RenderTargetLayout rt{};
				rt.format = render::TextureFormat::SRGBA32;
				rt.depthFormat = render::TextureFormat::D24S8;
				rt.bUseMSAA = true;

				viewportTexture = core::SObject::Create<render::RenderTexture>(rt);
				viewportTexture->SetUUID(core::UUID{ "180635b4e4d1a98ebb0064ab47dc452a" });
				viewportTexture->Build(*renderer.GetContext());
			}
			else
				viewportTexture = viewportPtr;

			game::GameObject* camObj = AddGameObject("EditorCamera");
			camObj->transform->SetPosition({ 2.f, 2.f, 2.f });
			camObj->hideInspector = true;
			camObj->bNotSave = true;
			editorCamera = camObj->AddComponent<game::EditorCamera>();
			editorCamera->SetRenderTexture(viewportTexture);
			editorCamera->GetNative().SetActive(true);
			editorRenderer->SetEditorCamera(editorCamera->GetNative());

			auto PickingCamObj = AddGameObject("PickingCamera");
			PickingCamObj->bNotSave = true;
			PickingCamObj->transform->SetParent(camObj->transform);
			PickingCamObj->transform->SetPosition({ 0.f, 0.f, 0.f });
			pickingCamera = PickingCamObj->AddComponent<game::PickingCamera>();
			pickingCamera->SetFollowCamera(editorCamera);
			pickingCamera->GetNative().SetActive(true);

			editorRenderer->SetPickingCamera(pickingCamera->GetNative());

			auto outlineTexture = EditorResource::GetInstance()->GetTexture("OutlineTexture");
			assert(outlineTexture);
			editorRenderer->GetOutlinePass()->SetOutTexture(static_cast<render::RenderTexture&>(*outlineTexture));

			auto outlinePostMat = EditorResource::GetInstance()->GetMaterial("OutlinePostMaterial");
			assert(outlinePostMat);
			editorRenderer->GetPostOutlinePass()->SetOutlineMaterial(*outlinePostMat);

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
		}

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

	SH_EDITOR_API auto EditorWorld::GetEditorUI() const -> EditorUI&
	{
		return *editorUI;
	}

	SH_EDITOR_API auto EditorWorld::GetViewportTexture() const -> render::RenderTexture&
	{
		return *viewportTexture;
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
		core::Json mainJson = Super::Serialize();

		core::Json objsJson = core::Json::array();
		core::Json editorObjsJson = core::Json::array();
		for (auto obj : objs)
		{
			if (obj->bNotSave || !core::IsValid(obj))
				continue;
			if (obj->bEditorOnly)
				editorObjsJson.push_back(obj->Serialize());
			else
				objsJson.push_back(obj->Serialize());
		}
		mainJson["objs"] = std::move(objsJson);
		mainJson["editorObjs"] = std::move(editorObjsJson);
		if (core::IsValid(editorCamera))
		{
			const game::Vec3& camPos = editorCamera->gameObject.transform->GetWorldPosition();
			mainJson["camPos"] = { camPos.x ,camPos.y, camPos.z };
			mainJson["cam"] = editorCamera->Serialize();
		}
		return mainJson;
	}
	SH_EDITOR_API void EditorWorld::Deserialize(const core::Json& json)
	{
		// 에디터Only 오브젝트들을 objs로 옮겨서 에디터상에선 역직렬화 되게
		core::Json copyJson = json;
		for (core::Json& editorObjJson : copyJson["editorObjs"])
			copyJson["objs"].push_back(std::move(editorObjJson));

		Super::Deserialize(copyJson);

		if (editorCamera != nullptr)
		{
			if (json.contains("camPos") && json["camPos"].is_array())
			{
				const auto& camPosJson = json["camPos"];
				editorCamera->gameObject.transform->SetWorldPosition(camPosJson[0], camPosJson[1], camPosJson[2]);
				editorCamera->gameObject.transform->UpdateMatrix();
			}
			if (json.contains("cam"))
				editorCamera->Deserialize(json["cam"]);
		}
	}
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
}//namespace