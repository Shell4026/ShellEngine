#include "EditorWorld.h"
#include "EditorResource.h"
#include "EditorUI.h"

#include "Render/VulkanImpl/VulkanContext.h"
#include "Render/VulkanImpl/VulkanShaderPass.h"
#include "Render/VulkanImpl/VulkanShaderPassBuilder.h"
#include "Render/Mesh/Plane.h"
#include "Render/Mesh/Grid.h"
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
	SH_EDITOR_API EditorWorld::EditorWorld(render::Renderer& renderer, const game::ComponentModule& module, game::ImGUImpl& guiContext) :
		World(renderer, module), guiContext(guiContext)
	{
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

	SH_EDITOR_API void EditorWorld::InitResource()
	{
		Super::InitResource();

		EditorResource::GetInstance()->LoadAllAssets(*this);

		render::vk::VulkanShaderPassBuilder shaderBuilder{ static_cast<sh::render::vk::VulkanContext&>(*renderer.GetContext()) };

		ShaderLoader loader{ &shaderBuilder };
		TextureLoader texLoader{ *renderer.GetContext() };
		ModelLoader modelLoader{ *renderer.GetContext() };

		auto defaultShader = shaders.AddResource("DefaultShader", loader.LoadShader("shaders/default.shader"));
		auto lineShader = shaders.AddResource("Line", loader.LoadShader("shaders/line.shader"));
		auto errorShader = shaders.AddResource("ErrorShader", loader.LoadShader("shaders/error.shader"));
		auto gridShader = shaders.AddResource("GridShader", loader.LoadShader("shaders/grid.shader"));
		auto pickingShader = shaders.AddResource("PickingShader", loader.LoadShader("shaders/picking.shader"));
		auto outlineShader = shaders.AddResource("OutlineShader", loader.LoadShader("shaders/outline.shader"));
		auto triangleShader = shaders.AddResource("TriangleShader", loader.LoadShader("shaders/triangle.shader"));

		auto errorMat = materials.AddResource("ErrorMaterial", render::Material{ outlineShader });
		auto lineMat = materials.AddResource("LineMaterial", render::Material{ lineShader });
		auto gridMat = materials.AddResource("GridMaterial", render::Material{ gridShader });
		auto pickingMat = materials.AddResource("PickingMaterial", render::Material{ pickingShader });
		auto triMat = materials.AddResource("TriangleMaterial", render::Material{ triangleShader });

		auto plane = meshes.AddResource("PlaneMesh", render::Plane{});
		auto grid = meshes.AddResource("GridMesh", render::Grid{});

		errorShader->SetUUID(core::UUID{ "bbc4ef7ec45dce223297a224f8093f0f" });
		defaultShader->SetUUID(core::UUID{ "ad9217609f6c7e0f1163785746cc153e" });

		errorMat->SetProperty("outlineWidth", 0.2f);
		errorMat->SetProperty("color", glm::vec4{ 1.0f, 0.f, 0.f, 1.0f });
		errorMat->Build(*renderer.GetContext());
		pickingMat->Build(*renderer.GetContext());

		lineMat->Build(*renderer.GetContext());

		gridMat->SetProperty("color", glm::vec4{ 0.6f, 0.6f, 0.8f, 0.2f });
		gridMat->Build(*renderer.GetContext());
		
		triMat->SetProperty("color", glm::vec3{ 0.f, 1.f, 0.f });
		triMat->Build(*renderer.GetContext());

		plane->Build(*renderer.GetContext());
		grid->Build(*renderer.GetContext());

		render::RenderTexture* viewportTexture = core::SObject::Create<render::RenderTexture>();
		viewportTexture->Build(*renderer.GetContext());
		textures.AddResource("Viewport", viewportTexture);

		render::RenderTexture* camTexture = core::SObject::Create<render::RenderTexture>();
		camTexture->SetSize(128, 128);
		camTexture->Build(*renderer.GetContext());
		textures.AddResource("CamTexture", camTexture);

		game::GameObject* camObj = AddGameObject("EditorCamera");
		camObj->transform->SetPosition({ 2.f, 2.f, 2.f });
		//camObj->hideInspector = true;
		editorCamera = camObj->AddComponent<game::EditorCamera>();
		editorCamera->SetUUID(core::UUID{ "61b7bc9f9fd2ca27dcbad8106745f62a" });
		editorCamera->SetRenderTexture(viewportTexture);
		this->SetMainCamera(editorCamera);

		game::GameObject* camObj2 = AddGameObject("Camera");
		camObj2->transform->SetPosition({ -2.f, 2.f, -2.f });
		camObj2->AddComponent<game::Camera>()->SetRenderTexture(camTexture);

		auto PickingCamObj = AddGameObject("PickingCamera");
		PickingCamObj->transform->SetParent(camObj->transform);
		PickingCamObj->transform->SetPosition({ 0.f, 0.f, 0.f });
		auto pickingCam = PickingCamObj->AddComponent<game::PickingCamera>();
		pickingCam->SetUUID(core::UUID{ "af9cac824334bcaccd86aad8a18e3cba" });
		pickingCam->SetFollowCamera(editorCamera);

		// 렌더 테스트용 객체
		//auto prop = core::SObject::Create<render::MaterialPropertyBlock>();
		//prop->SetProperty("offset", glm::vec2{ 0.5f, 0.f });

		//game::GameObject* tri = AddGameObject("Triangle");
		//tri->transform->SetScale(0.5f, 1.0f, 1.f);
		//auto m = tri->AddComponent<game::MeshRenderer>();
		//m->SetMaterial(triMat);
		//m->SetMesh(plane);
		//m->SetMaterialPropertyBlock(prop);
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
		editorUI = std::make_unique<editor::EditorUI>(*this, guiContext);

		auto grid = this->AddGameObject("Grid");
		grid->hideInspector = true;
		auto meshRenderer = grid->AddComponent<game::MeshRenderer>();
		meshRenderer->SetMesh(this->meshes.GetResource("GridMesh"));
		meshRenderer->SetMaterial(this->materials.GetResource("GridMaterial"));

		auto axis = AddGameObject("Axis");
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