#include "EditorWorld.h"
#include "EditorResource.h"
#include "EditorPickingPass.h"
#include "EditorOutlinePass.h"
#include "EditorPostOutlinePass.h"

#include "Component/EditorUI.h"
#include "Component/OutlineComponent.h"

#include "Core/Name.h"

#include "Render/VulkanImpl/VulkanContext.h"
#include "Render/VulkanImpl/VulkanShaderPass.h"
#include "Render/VulkanImpl/VulkanShaderPassBuilder.h"
#include "Render/Mesh/Plane.h"
#include "Render/Mesh/Grid.h"
#include "Render/Model.h"
#include "Render/MaterialPropertyBlock.h"

#include "Game/GameObject.h"
#include "Game/Component/MeshRenderer.h"
#include "Game/Component/EditorCamera.h"
#include "Game/Component/PickingCamera.h"
#include "Game/Component/PickingRenderer.h"
#include "Game/Component/EditorControl.h"
#include "Game/Component/LineRenderer.h"

#include "ModelLoader.h"
#include "TextureLoader.h"
#include "ShaderLoader.h"

namespace sh::editor
{
	void EditorWorld::AddOrDestroyOutlineComponent(game::GameObject& obj, bool bAdd)
	{
		for (auto child : obj.transform->GetChildren())
			AddOrDestroyOutlineComponent(child->gameObject, bAdd);

		auto meshRenderer = obj.GetComponent<game::MeshRenderer>();
		if (!core::IsValid(meshRenderer))
			return;

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

	SH_EDITOR_API EditorWorld::EditorWorld(render::Renderer& renderer, const game::ComponentModule& module, game::ImGUImpl& guiContext) :
		World(renderer, module, guiContext)
	{
		pickingPass = renderer.AddRenderPipeline<EditorPickingPass>();
		outlinePass = renderer.AddRenderPipeline<EditorOutlinePass>();

		onComponentAddListener.SetCallback([&](game::Component* component)
			{
				game::GameObject* obj = &component->gameObject;
				onComponentAdd.Notify(obj, component);
				if (obj == grid || obj == axis)
					return;
				if (component->GetType() == game::MeshRenderer::GetStaticType())
				{
					auto pickingRenderer = obj->AddComponent<game::PickingRenderer>();
					pickingRenderer->hideInspector = true;
					pickingRenderer->SetCamera(*pickingCamera);
					pickingRenderer->SetMeshRenderer(static_cast<game::MeshRenderer&>(*component));
				}
			}
		);
		ImGui::SetCurrentContext(guiContext.GetContext());
	}

	SH_EDITOR_API EditorWorld::~EditorWorld()
	{
	}

	SH_EDITOR_API void EditorWorld::Clean()
	{
		Super::Clean();
		editorUI = nullptr;
		selected = nullptr;
	}

	SH_EDITOR_API void EditorWorld::InitResource()
	{
		Super::InitResource();

		EditorResource::GetInstance()->LoadAllAssets(*this);

		postOutlinePass = renderer.AddRenderPipeline<EditorPostOutlinePass>();

		render::vk::VulkanShaderPassBuilder shaderBuilder{ static_cast<sh::render::vk::VulkanContext&>(*renderer.GetContext()) };

		ShaderLoader loader{ &shaderBuilder };
		TextureLoader texLoader{ *renderer.GetContext() };
		ModelLoader modelLoader{ *renderer.GetContext() };

		auto defaultShader = shaders.AddResource("DefaultShader", loader.LoadShader("shaders/default.shader"));
		auto lineShader = shaders.AddResource("Line", loader.LoadShader("shaders/line.shader"));
		auto errorShader = shaders.AddResource("ErrorShader", loader.LoadShader("shaders/error.shader"));
		auto gridShader = shaders.AddResource("GridShader", loader.LoadShader("shaders/grid.shader"));
		auto pickingShader = shaders.AddResource("EditorPickingShader", loader.LoadShader("shaders/EditorPicking.shader"));
		auto outlineShader = shaders.AddResource("OutlineShader", loader.LoadShader("shaders/outline.shader"));
		auto triangleShader = shaders.AddResource("TriangleShader", loader.LoadShader("shaders/triangle.shader"));

		auto outlinePreShader = shaders.AddResource("OutlinePreShader", loader.LoadShader("shaders/EditorOutlinePre.shader"));
		auto outlinePostShader = shaders.AddResource("OutlinePostShader", loader.LoadShader("shaders/EditorOutlinePost.shader"));

		auto errorMat = materials.AddResource("ErrorMaterial", render::Material{ errorShader });
		auto lineMat = materials.AddResource("LineMaterial", render::Material{ lineShader });
		auto gridMat = materials.AddResource("GridMaterial", render::Material{ gridShader });
		auto pickingMat = materials.AddResource("PickingMaterial", render::Material{ pickingShader });
		auto triMat = materials.AddResource("TriangleMaterial", render::Material{ triangleShader });
		auto outlinePreMat = materials.AddResource("OutlinePreMaterial", render::Material{ outlinePreShader });
		auto outlinePostMat = materials.AddResource("OutlinePostMaterial", render::Material{ outlinePostShader });

		auto plane = meshes.AddResource("PlaneMesh", render::Plane{});
		auto grid = meshes.AddResource("GridMesh", render::Grid{});

		auto boxModel = modelLoader.Load("model/cube.obj");
		for (auto mesh : boxModel->GetMeshes())
			meshes.AddResource("BoxMesh", mesh);

		errorShader->SetUUID(core::UUID{ "bbc4ef7ec45dce223297a224f8093f0f" });
		defaultShader->SetUUID(core::UUID{ "ad9217609f6c7e0f1163785746cc153e" });

		render::RenderTexture* viewportTexture = core::SObject::Create<render::RenderTexture>(render::Texture::TextureFormat::SRGBA32);
		viewportTexture->Build(*renderer.GetContext());
		textures.AddResource("Viewport", viewportTexture);

		render::RenderTexture* outlineTexture = core::SObject::Create<render::RenderTexture>(render::Texture::TextureFormat::R8);
		outlineTexture->SetSize(1024, 1024);
		outlineTexture->Build(*renderer.GetContext());
		textures.AddResource("OutlineTexture", outlineTexture);

		errorMat->Build(*renderer.GetContext());
		pickingMat->Build(*renderer.GetContext());
		lineMat->Build(*renderer.GetContext());

		gridMat->SetProperty("color", glm::vec4{ 0.6f, 0.6f, 0.8f, 0.2f });
		gridMat->Build(*renderer.GetContext());
		
		triMat->SetProperty("color", glm::vec3{ 0.f, 1.f, 0.f });
		triMat->Build(*renderer.GetContext());

		plane->Build(*renderer.GetContext());
		grid->Build(*renderer.GetContext());

		outlinePreMat->Build(*renderer.GetContext());

		outlinePostMat->SetProperty("outlineWidth", 1.0f);
		outlinePostMat->SetProperty("outlineColor", glm::vec4{ 41 / 255.0f, 74 / 255.0f, 122 / 255.0f, 1.0f });
		outlinePostMat->SetProperty("tex", outlineTexture);
		outlinePostMat->Build(*renderer.GetContext());

		game::GameObject* camObj = AddGameObject("EditorCamera");
		camObj->transform->SetPosition({ 2.f, 2.f, 2.f });
		camObj->hideInspector = true;
		camObj->bNotSave = true;
		editorCamera = camObj->AddComponent<game::EditorCamera>();
		editorCamera->SetUUID(core::UUID{ "61b7bc9f9fd2ca27dcbad8106745f62a" });
		editorCamera->SetRenderTexture(viewportTexture);
		this->SetMainCamera(editorCamera);

		auto PickingCamObj = AddGameObject("PickingCamera");
		PickingCamObj->bNotSave = true;
		PickingCamObj->transform->SetParent(camObj->transform);
		PickingCamObj->transform->SetPosition({ 0.f, 0.f, 0.f });
		pickingCamera = PickingCamObj->AddComponent<game::PickingCamera>();
		pickingCamera->SetUUID(core::UUID{ "af9cac824334bcaccd86aad8a18e3cba" });
		pickingCamera->SetFollowCamera(editorCamera);

		pickingPass->SetCamera(pickingCamera->GetNative());
		outlinePass->SetCamera(*editorCamera);
		outlinePass->SetOutTexture(*outlineTexture);

		postOutlinePass->SetCamera(*editorCamera);
		postOutlinePass->SetOutlineMaterial(*outlinePostMat);

		renderer.GetRenderPipeline(core::Name{ "Forward" })->IgnoreCamera(pickingCamera->GetNative());

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
	}

	SH_EDITOR_API void EditorWorld::SetSelectedObject(core::SObject* obj)
	{
		if (obj == nullptr)
		{
			for (auto selected : selectedObjs)
			{
				if (core::IsValid(selected) && selected->GetType() == game::GameObject::GetStaticType())
				{
					if (core::IsValid(selected))
					{
						game::GameObject* selectedObj = static_cast<game::GameObject*>(selected);
						auto control = selectedObj->GetComponent<game::EditorControl>();
						if (core::IsValid(control))
							control->Destroy();
						AddOrDestroyOutlineComponent(*selectedObj, false);
					}
				}
			}
			selectedObjs.clear();
		}

		if (selected == obj)
			return;

		if (core::IsValid(selected) && selected->GetType() == game::GameObject::GetStaticType())
		{
			game::GameObject* selectedObj = static_cast<game::GameObject*>(selected);
			auto control = selectedObj->GetComponent<game::EditorControl>();
			if (core::IsValid(control))
				control->Destroy();
			AddOrDestroyOutlineComponent(*selectedObj, false);
		}
		selected = obj;
		if (core::IsValid(selected) && selected->GetType() == game::GameObject::GetStaticType())
		{
			game::GameObject* selectedObj = static_cast<game::GameObject*>(selected);
			auto control = selectedObj->GetComponent<game::EditorControl>();
			if (!core::IsValid(control))
			{
				auto control = selectedObj->AddComponent<game::EditorControl>();
				control->SetCamera(editorCamera);
				control->hideInspector = true;
			}
			AddOrDestroyOutlineComponent(*selectedObj, true);
		}
	}
	SH_EDITOR_API auto EditorWorld::GetSelectedObject() const -> core::SObject*
	{
		return selected;
	}

	SH_EDITOR_API void EditorWorld::AddSelectedObject(core::SObject* obj)
	{
		selectedObjs.insert(obj);
		if (core::IsValid(obj) && obj->GetType() == game::GameObject::GetStaticType())
		{
			game::GameObject* selectedObj = static_cast<game::GameObject*>(obj);
			auto control = selectedObj->GetComponent<game::EditorControl>();
			if (!core::IsValid(control))
			{
				control = selectedObj->AddComponent<game::EditorControl>();
				control->SetCamera(editorCamera);
				control->hideInspector = true;
			}
			AddOrDestroyOutlineComponent(*selectedObj, true);
		}
	}

	SH_EDITOR_API auto EditorWorld::AddGameObject(std::string_view name) -> game::GameObject*
	{
		auto obj = Super::AddGameObject(name);
		obj->onComponentAdd.Register(onComponentAddListener);
		
		return obj;
	}

	SH_EDITOR_API auto EditorWorld::GetEditorUI() const -> EditorUI&
	{
		return *editorUI;
	}

	SH_EDITOR_API void EditorWorld::Start()
	{
		grid = AddGameObject("Grid"); // Grid와 Axis는 피킹 렌더러가 추가 되면 안 된다.
		grid->hideInspector = true;
		grid->bNotSave = true;
		auto meshRenderer = grid->AddComponent<game::MeshRenderer>();
		meshRenderer->SetMesh(this->meshes.GetResource("GridMesh"));
		meshRenderer->SetMaterial(this->materials.GetResource("GridMaterial"));

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