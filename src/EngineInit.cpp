#include "EngineInit.h"

#include "Core/Logger.h"
#include "Core/GarbageCollection.h"

#include "Window/Window.h"

#include "Render/VulkanRenderer.h"
#include "Render/VulkanShader.h"

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
		componentModule(nullptr), gc(nullptr),
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

		auto shader = world->shaders.AddResource("Triangle", loader.LoadShader<render::VulkanShader>
			("shaders/vert.spv", "shaders/frag.spv"));
		auto lineShader = world->shaders.AddResource("Line", loader.LoadShader<render::VulkanShader>
			("shaders/lineVert.spv", "shaders/lineFrag.spv"));
		auto mat = world->materials.AddResource("Material", sh::render::Material{ shader });
		auto mat2 = world->materials.AddResource("Material2", sh::render::Material{ shader });
		auto lineMat = world->materials.AddResource("LineMat", sh::render::Material{ lineShader });
		auto mesh = world->meshes.AddResource("Mesh", sh::render::Mesh{});
		auto mesh2 = world->meshes.AddResource("Mesh2", modelLoader.Load("model/test.obj"));
		auto tex = world->textures.AddResource("Texture0", texLoader.Load("textures/버터고양이.jpg"));
		auto tex2 = world->textures.AddResource("Texture1", texLoader.Load("textures/cat.jpg"));
		auto tex3 = world->textures.AddResource("Texture2", texLoader.Load("textures/viking_room.png"));

		shader->AddAttribute<glm::vec2>("uvs", 1);

		shader->AddUniform<glm::mat4>("model", 0, sh::render::Shader::ShaderStage::Vertex);
		shader->AddUniform<glm::mat4>("view", 0, sh::render::Shader::ShaderStage::Vertex);
		shader->AddUniform<glm::mat4>("proj", 0, sh::render::Shader::ShaderStage::Vertex);
		shader->AddUniform<glm::vec3>("offset1", 1, sh::render::Shader::ShaderStage::Vertex);
		shader->AddUniform<float>("offset2", 1, sh::render::Shader::ShaderStage::Vertex);
		shader->AddUniform<sh::render::Texture>("tex", 2, sh::render::Shader::ShaderStage::Fragment);
		shader->Build();

		lineShader->AddUniform<glm::mat4>("model", 0, sh::render::Shader::ShaderStage::Vertex);
		lineShader->AddUniform<glm::mat4>("view", 0, sh::render::Shader::ShaderStage::Vertex);
		lineShader->AddUniform<glm::mat4>("proj", 0, sh::render::Shader::ShaderStage::Vertex);
		lineShader->AddUniform<glm::vec4>("color", 1, sh::render::Shader::ShaderStage::Fragment);
		lineShader->SetTopology(sh::render::Shader::Topology::Line);
		lineShader->Build();

		mat->SetVector("offset1", glm::vec4(0.f, 0.0f, 0.f, 0.f));
		mat->SetFloat("offset2", 0.f);
		mat->SetTexture("tex", tex);

		mat2->SetTexture("tex", tex3);

		lineMat->SetVector("color", glm::vec4{ 1.0f, 1.0f, 0.0f, 1.0f });

		mesh->SetVertex({
			{-0.5f, 0.0f, -0.5f},
			{-0.5f, 0.0f, 0.5f},
			{0.5f, 0.0f, 0.5f},
			{0.5f, 0.0f, -0.5f}
			});
		mesh->SetIndices({
			0, 1, 2, 2, 3, 0
			});
		mesh->SetAttribute(sh::render::ShaderAttribute<glm::vec2>{"uvs", {
			{0.0f, 0.0f},
			{0.0f, 1.0f},
			{1.0f, 1.0f},
			{1.0f, 0.0f}
			}});
		mesh->Build(*renderer);

		GameObject* obj = world->AddGameObject("Test");
		GameObject* obj2 = world->AddGameObject("Test2");
		GameObject* obj3 = world->AddGameObject("Empty");

		//obj2->AddComponent(componentModule->GetComponent("ComponentTest")->New());

		auto transform = obj->transform;
		transform->SetRotation({ -90.f, 0.f, 0.f });

		auto meshRenderer = obj->AddComponent<MeshRenderer>();
		meshRenderer->SetMesh(*mesh2);
		meshRenderer->SetMaterial(*mat2);

		obj->AddComponent<UniformTest>();

		auto meshRenderer2 = obj2->AddComponent<MeshRenderer>();
		meshRenderer2->SetMesh(*mesh);
		meshRenderer2->SetMaterial(*mat);

		GameObject* cam = world->AddGameObject("Camera");
		cam->transform->SetPosition(glm::vec3(2.f, 2.f, 2.f));
		GameObject* cam2 = world->AddGameObject("Camera2");
		cam2->transform->SetPosition(glm::vec3(-2.f, 2.f, -2.f));
		Camera* cameraComponent2 = cam2->AddComponent<Camera>();
		cameraComponent2->SetRenderTexture(editorUI->GetViewport().GetRenderTexture());
#if SH_EDITOR
		Camera* cameraComponent = cam->AddComponent<EditorCamera>();
		cameraComponent->SetRenderTexture(editorUI->GetViewport().GetRenderTexture());
		cameraComponent2->SetDepth(1);
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
							renderer->SetViewport({ 150.f, 0.f }, { width - 150.f, height - 180 }); 
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

		gui = std::make_unique<game::ImGUImpl>(*window, static_cast<render::VulkanRenderer&>(*renderer));
		gui->Init();

#if SH_EDITOR
		editorUI = std::make_unique<editor::EditorUI>(*world, *gui);
#endif

		InitResource();

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