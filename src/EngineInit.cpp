#include "EngineInit.h"

#include "Core/Logger.h"
#include "Core/GarbageCollection.h"

#include "Window/Window.h"

#include "Render/VulkanImpl/VulkanRenderer.h"
#include "Render/VulkanImpl/VulkanShader.h"
#include "Render/Mesh/Plane.h"
#include "Render/IUniformBuffer.h"

#include "Game/VulkanShaderBuilder.h"
#include "Game/ShaderLoader.h"
#include "Game/TextureLoader.h"
#include "Game/ModelLoader.h"
#include "Game/GameObject.h"
#include "Game/ImGUImpl.h"
#include "Game/RenderThread.h"
#include "Game/Input.h"

#include "Game/ComponentModule.h"
#include "Game/Component/LineRenderer.h"
#include "Game/Component/EditorCamera.h"
#include "Game/Component/UniformTest.h"

#if SH_EDITOR
#include "Editor/EditorUI.h"
#endif

#include "fmt/core.h"

namespace sh
{
	EngineInit::EngineInit() :
		moduleLoader(), 
		componentModule(nullptr), gc(nullptr), world(nullptr),
		gameThread(nullptr), renderThread(nullptr),
		limitFps(144),
		bStop(false)
	{
	}
	EngineInit::~EngineInit()
	{
		Clean();
	}
	void EngineInit::Clean()

	{
		SH_INFO("Engine shutdown");
		world->Clean();
		world->Destroy();
#if SH_EDITOR
		editorUI.reset();
#endif
		gui.reset();
		gc->Update();
		renderer.reset();
		window.reset();
	}

	inline void EngineInit::LoadModule()
	{
		SH_INFO("Module loading");
		void* modulePtr = moduleLoader.Load("ShellEngineUser");
		componentModule = reinterpret_cast<sh::game::ComponentModule*>(modulePtr);
		if (componentModule == nullptr)
			throw std::runtime_error{ "Can't load user module!" };

		componentModule->RegisterComponent<game::LineRenderer>("LineRenderer");
		for (auto& components : componentModule->GetComponents())
		{
			SH_INFO(fmt::format("Load Component: {}\n", components.first));
		}
	}

	inline void EngineInit::InitResource()
	{
		SH_INFO("Resource initialization");
		using namespace sh::game;

		VulkanShaderBuilder shaderBuilder{ static_cast<render::VulkanRenderer&>(*renderer)};

		ShaderLoader loader{ &shaderBuilder };
		TextureLoader texLoader{ *renderer };
		ModelLoader modelLoader{ *renderer };

		auto defaultShader = world->shaders.AddResource("Default", loader.LoadShader<render::VulkanShader>
			("shaders/default.vert.spv", "shaders/default.frag.spv"));
		auto lineShader = world->shaders.AddResource("Line", loader.LoadShader<render::VulkanShader>("shaders/line.vert.spv", "shaders/line.frag.spv"));
		auto mat = world->materials.AddResource("Material", sh::render::Material{ defaultShader });
		auto catMat0 = world->materials.AddResource("Material2", sh::render::Material{ defaultShader });
		auto catMat1 = world->materials.AddResource("Material3", sh::render::Material{ defaultShader });
		auto lineMat = world->materials.AddResource("LineMat", sh::render::Material{ lineShader });
		auto plane = world->meshes.AddResource("Plane", sh::render::Plane{});
		auto cube = world->meshes.AddResource("Cube", modelLoader.Load("model/cube.obj"));
		auto mesh2 = world->meshes.AddResource("Mesh2", modelLoader.Load("model/test.obj"));
		auto catTex0 = world->textures.AddResource("Texture0", texLoader.Load("textures/버터고양이.jpg"));
		auto catTex1 = world->textures.AddResource("Texture1", texLoader.Load("textures/cat.jpg"));
		auto tex = world->textures.AddResource("Texture2", texLoader.Load("textures/viking_room.png"));
		world->textures.AddResource("FolderIcon", texLoader.Load("textures/folder.png"));

		defaultShader->AddAttribute<glm::vec2>("uvs", 1);

		auto ObjectUniformType = render::Shader::UniformType::Object;
		auto MaterialUniformType = render::Shader::UniformType::Material;

		defaultShader->AddUniform<glm::mat4>("model", ObjectUniformType, 0, sh::render::Shader::ShaderStage::Vertex);
		defaultShader->AddUniform<glm::mat4>("view", ObjectUniformType, 0, sh::render::Shader::ShaderStage::Vertex);
		defaultShader->AddUniform<glm::mat4>("proj", ObjectUniformType, 0, sh::render::Shader::ShaderStage::Vertex);

		defaultShader->AddUniform<sh::render::Texture>("tex", MaterialUniformType, 0, sh::render::Shader::ShaderStage::Fragment);
		defaultShader->Build();

		lineShader->AddUniform<glm::mat4>("model", ObjectUniformType, 0, sh::render::Shader::ShaderStage::Vertex);
		lineShader->AddUniform<glm::mat4>("view", ObjectUniformType, 0, sh::render::Shader::ShaderStage::Vertex);
		lineShader->AddUniform<glm::mat4>("proj", ObjectUniformType, 0, sh::render::Shader::ShaderStage::Vertex);

		lineShader->AddUniform<glm::vec3>("start", MaterialUniformType, 0, sh::render::Shader::ShaderStage::Vertex);
		lineShader->AddUniform<glm::vec3>("end", MaterialUniformType, 0, sh::render::Shader::ShaderStage::Vertex);
		lineShader->AddUniform<glm::vec4>("color", MaterialUniformType, 1, sh::render::Shader::ShaderStage::Fragment);
		lineShader->Build();

		catMat0->SetTexture("tex", catTex0);
		catMat0->Build(*renderer);
		catMat1->SetTexture("tex", catTex1);
		catMat1->Build(*renderer);

		lineMat->SetVector("start", glm::vec4(0.f));
		lineMat->SetVector("end", glm::vec4(0.f));
		lineMat->SetVector("color", glm::vec4{ 1.0f, 1.0f, 0.0f, 1.0f });
		lineMat->Build(*renderer);

		mat->SetTexture("tex", tex);
		mat->Build(*renderer);

		plane->Build(*renderer);
		cube->Build(*renderer);

		GameObject* obj = world->AddGameObject("Test");
		GameObject* obj2 = world->AddGameObject("Test2");
		GameObject* obj3 = world->AddGameObject("Empty");

		//obj2->AddComponent(componentModule->GetComponent("ComponentTest")->New());

		auto transform = obj->transform;
		transform->SetRotation({ 0.f, 0.f, 0.f });
		transform->SetPosition(0, 0, -1);
		transform->SetScale(1.f);

		obj2->transform->SetPosition(1, 1, 0);
		obj2->transform->SetParent(obj->transform);

		obj->AddComponent<UniformTest>();
		auto meshRenderer = obj->AddComponent<MeshRenderer>();
		meshRenderer->SetMesh(*cube);
		meshRenderer->SetMaterial(*catMat0);

		meshRenderer = obj2->AddComponent<MeshRenderer>();
		meshRenderer->SetMesh(*cube);
		meshRenderer->SetMaterial(*catMat1);

		meshRenderer = obj3->AddComponent<MeshRenderer>();
		meshRenderer->SetMesh(*cube);
		meshRenderer->SetMaterial(*mat);

		GameObject* cam = world->AddGameObject("Camera");
		cam->transform->SetPosition(glm::vec3(2.f, 2.f, 2.f));
		//GameObject* cam2 = world->AddGameObject("Camera2");
		//cam2->transform->SetPosition(glm::vec3(-2.f, 2.f, -2.f));
		//Camera* cameraComponent2 = cam2->AddComponent<Camera>();
		//cameraComponent2->SetRenderTexture(editorUI->GetViewport().GetRenderTexture());
#if SH_EDITOR
		Camera* cameraComponent = cam->AddComponent<EditorCamera>();
		//cameraComponent->SetDepth(2);
#endif

		world->SetMainCamera(cameraComponent);
	}

	void EngineInit::ProcessInput()
	{
		game::Input::Update();

		sh::window::Event e;
		while (window->PollEvent(e))
		{
			game::Input::UpdateEvent(e);
			gui->ProcessEvent(e);
			switch (e.type)
			{
			case sh::window::Event::EventType::Close:
				bStop = true;
				break;
			case sh::window::Event::EventType::Resize:
				if (window->width == 0)
				{
					renderer->Pause(true);
				}
				else
				{
					renderer->Pause(false);
					renderThread->AddTaskFromOtherThread(
						[&, width = window->width, height = window->height]
						{
							renderer->SetViewport({ 0, 0.f }, { width, height }); 
						}
					);
					gui->Resize();
				}
				break;
			case sh::window::Event::EventType::WindowFocus:
				window->SetFps(limitFps);
				std::cout << "FocusIn\n";
				break;
			case sh::window::Event::EventType::WindowFocusOut:
				window->SetFps(30);
				std::cout << "FocusOut\n";
				break;
			}
		}
	}

	void EngineInit::Start()
	{
		SH_INFO("Engine start");
		LoadModule();

		gc = core::GarbageCollection::GetInstance(); //GC 초기화

		SH_INFO("Window initialization");
		window = std::make_unique<window::Window>();
		window->Create(u8"ShellEngine", 1024, 768, sh::window::Window::Style::Resize);
		window->SetFps(limitFps);

		SH_INFO("Renderer initialization");
		renderer = std::make_unique<sh::render::VulkanRenderer>(threadSyncManager);
		renderer->Init(*window);
		renderer->SetViewport({ 150.f, 0.f }, { window->width - 150.f, window->height - 180 });

		world = core::SObject::Create<game::World>(*renderer.get(), *componentModule);
		gc->SetRootSet(world);

		InitResource();

		gui = std::make_unique<game::ImGUImpl>(*window, static_cast<render::VulkanRenderer&>(*renderer));
		gui->Init();

#if SH_EDITOR
		editorUI = std::make_unique<editor::EditorUI>(*world, *gui);
		world->GetMainCamera()->SetRenderTexture(editorUI->GetViewport().GetRenderTexture());
#endif

		SH_INFO("Start world");
		world->Start();

		SH_INFO("Thread creation");
		renderThread = game::RenderThread::GetInstance();
		renderThread->SetWaitableThread(true);
		renderThread->Init(*renderer);
		threadSyncManager.AddThread(*renderThread);

		SH_INFO("Start loop");
		Loop();
		SH_INFO("End loop");

		renderThread->Stop();
		renderThread->GetThread().join();
	}

	inline void EngineInit::Loop()
	{
		while (!bStop)
		{
			window->ProcessFrame();

			std::string deltaTime = std::to_string(window->GetDeltaTime());
			deltaTime.erase(deltaTime.begin() + 5, deltaTime.end());
			window->SetTitle("ShellEngine [DeltaTime:" + deltaTime + "ms]");
			ProcessInput();
			if (bStop)
				return;

			this->world->Update(window->GetDeltaTime());
			this->gui->Begin();
#if SH_EDITOR
			this->editorUI->Update();
#endif
			this->gui->End();

			gc->Update();
			threadSyncManager.Sync();
			
			threadSyncManager.AwakeThread();
		}
	}
}