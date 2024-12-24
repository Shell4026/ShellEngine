#include "EngineInit.h"

#include "Core/Logger.h"
#include "Core/GarbageCollection.h"
#include "Core/FileSystem.h"

#include "Window/Window.h"

#include "Render/VulkanImpl/VulkanRenderer.h"
#include "Render/VulkanImpl/VulkanShader.h"
#include "Render/IUniformBuffer.h"
#include "Render/Mesh/Plane.h"
#include "Render/Mesh/Grid.h"

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

#if SH_EDITOR
#include "Editor/EditorUI.h"
#include "Editor/EditorWorld.h"
#include "Editor/AssetDatabase.h"
#endif

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
		gc->Update();
		gc->ForceDelete(world);

		gui.reset();

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

		for (auto& components : componentModule->GetComponents())
		{
			SH_INFO(fmt::format("Load Component: {}\n", components.first));
		}
	}

	inline void EngineInit::InitResource()
	{
		SH_INFO("Resource initialization");
		using namespace sh::game;

		VulkanShaderBuilder shaderBuilder{ static_cast<render::vk::VulkanRenderer&>(*renderer)};

		ShaderLoader loader{ &shaderBuilder };
		TextureLoader texLoader{ *renderer };
		ModelLoader modelLoader{ *renderer };

		auto defaultShader = world->shaders.AddResource("Default", loader.LoadShader<render::VulkanShader>("shaders/default.vert.spv", "shaders/default.frag.spv"));
		auto lineShader = world->shaders.AddResource("Line", loader.LoadShader<render::VulkanShader>("shaders/line.vert.spv", "shaders/line.frag.spv"));
		auto errorShader = world->shaders.AddResource("ErrorShader", loader.LoadShader<render::VulkanShader>("shaders/error.vert.spv", "shaders/error.frag.spv"));
		auto gridShader = world->shaders.AddResource("GridShader", loader.LoadShader<render::VulkanShader>("shaders/grid.vert.spv", "shaders/grid.frag.spv"));
		auto pickingShader = world->shaders.AddResource("PickingShader", loader.LoadShader<render::VulkanShader>("shaders/picking.vert.spv", "shaders/picking.frag.spv"));

		auto errorMat = world->materials.AddResource("ErrorMaterial", sh::render::Material{ errorShader });
		auto lineMat = world->materials.AddResource("LineMaterial", sh::render::Material{ lineShader });
		auto gridMat = world->materials.AddResource("GridMaterial", sh::render::Material{ gridShader });
		auto pickingMat = world->materials.AddResource("PickingMaterial", sh::render::Material{ pickingShader });

		auto plane = world->meshes.AddResource("PlaneMesh", sh::render::Plane{});
		auto grid = world->meshes.AddResource("GridMesh", sh::render::Grid{});

		auto ObjectUniformType = render::Shader::UniformType::Object;
		auto MaterialUniformType = render::Shader::UniformType::Material;

		errorShader->SetUUID(core::UUID{ "bbc4ef7ec45dce223297a224f8093f0f" });
		errorShader->SetName("ErrorShader");
		errorShader->AddUniform<glm::mat4>("model", ObjectUniformType, 0, sh::render::Shader::ShaderStage::Vertex);
		errorShader->AddUniform<glm::mat4>("view", ObjectUniformType, 0, sh::render::Shader::ShaderStage::Vertex);
		errorShader->AddUniform<glm::mat4>("proj", ObjectUniformType, 0, sh::render::Shader::ShaderStage::Vertex);
		errorShader->Build();

		defaultShader->SetUUID(core::UUID{ "ad9217609f6c7e0f1163785746cc153e" });
		defaultShader->SetName("DefaultShader");
		defaultShader->AddAttribute<glm::vec2>("uvs", 1);
		defaultShader->AddAttribute<glm::vec3>("normals", 2);
		defaultShader->AddUniform<glm::mat4>("model", ObjectUniformType, 0, sh::render::Shader::ShaderStage::Vertex);
		defaultShader->AddUniform<glm::mat4>("view", ObjectUniformType, 0, sh::render::Shader::ShaderStage::Vertex);
		defaultShader->AddUniform<glm::mat4>("proj", ObjectUniformType, 0, sh::render::Shader::ShaderStage::Vertex);
		defaultShader->AddUniform<glm::vec4[10]>("lightPosRange", ObjectUniformType, 1, sh::render::Shader::ShaderStage::Fragment);
		defaultShader->AddUniform<int>("lightCount", ObjectUniformType, 1, sh::render::Shader::ShaderStage::Fragment);
		defaultShader->AddUniform<float>("ambient", MaterialUniformType, 0, sh::render::Shader::ShaderStage::Fragment);
		defaultShader->AddUniform<sh::render::Texture>("tex", MaterialUniformType, 1, sh::render::Shader::ShaderStage::Fragment);
		defaultShader->Build();

		lineShader->AddUniform<glm::mat4>("model", ObjectUniformType, 0, sh::render::Shader::ShaderStage::Vertex);
		lineShader->AddUniform<glm::mat4>("view", ObjectUniformType, 0, sh::render::Shader::ShaderStage::Vertex);
		lineShader->AddUniform<glm::mat4>("proj", ObjectUniformType, 0, sh::render::Shader::ShaderStage::Vertex);
		lineShader->AddUniform<glm::vec3>("start", ObjectUniformType, 1, sh::render::Shader::ShaderStage::Vertex);
		lineShader->AddUniform<glm::vec3>("end", ObjectUniformType, 1, sh::render::Shader::ShaderStage::Vertex);
		lineShader->AddUniform<glm::vec4>("color", ObjectUniformType, 2, sh::render::Shader::ShaderStage::Fragment);
		lineShader->AddUniform<glm::vec4>("test", MaterialUniformType, 0, sh::render::Shader::ShaderStage::Fragment);
		lineShader->Build();

		gridShader->AddUniform<glm::mat4>("model", ObjectUniformType, 0, sh::render::Shader::ShaderStage::Vertex);
		gridShader->AddUniform<glm::mat4>("view", ObjectUniformType, 0, sh::render::Shader::ShaderStage::Vertex);
		gridShader->AddUniform<glm::mat4>("proj", ObjectUniformType, 0, sh::render::Shader::ShaderStage::Vertex);
		gridShader->AddUniform<glm::vec4>("color", MaterialUniformType, 0, sh::render::Shader::ShaderStage::Fragment);
		gridShader->Build();

		pickingShader->AddUniform<glm::mat4>("model", ObjectUniformType, 0, sh::render::Shader::ShaderStage::Vertex);
		pickingShader->AddUniform<glm::mat4>("view", ObjectUniformType, 0, sh::render::Shader::ShaderStage::Vertex);
		pickingShader->AddUniform<glm::mat4>("proj", ObjectUniformType, 0, sh::render::Shader::ShaderStage::Vertex);
		pickingShader->AddUniform<glm::vec4>("id", ObjectUniformType, 1, sh::render::Shader::ShaderStage::Fragment);
		pickingShader->Build();

		errorMat->Build(*renderer);
		pickingMat->Build(*renderer);

		lineMat->SetVector("test", glm::vec4{ 1.f, 0.f, 0.f, 1.f });
		lineMat->Build(*renderer);

		gridMat->SetVector("color", glm::vec4{ 0.6f, 0.6f, 0.8f, 0.2f });
		gridMat->Build(*renderer);

		plane->Build(*renderer);
		grid->Build(*renderer);
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
					renderThread->AddBeginTaskFromOtherThread(
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

		SH_INFO_FORMAT("System thread count: {}", std::thread::hardware_concurrency());

		gc = core::GarbageCollection::GetInstance(); //GC 초기화

		SH_INFO("Window initialization");
		window = std::make_unique<window::Window>();
		window->Create(u8"ShellEngine", 1024, 768, sh::window::Window::Style::Resize);
		window->SetFps(limitFps);

		SH_INFO("Renderer initialization");
		renderer = std::make_unique<sh::render::vk::VulkanRenderer>(threadSyncManager);
		renderer->Init(*window);
		renderer->SetViewport({ 150.f, 0.f }, { window->width - 150.f, window->height - 180 });

		gui = std::make_unique<game::ImGUImpl>(*window, static_cast<render::vk::VulkanRenderer&>(*renderer));
		gui->Init();
#if SH_EDITOR
		world = core::SObject::Create<editor::EditorWorld>(*renderer, *componentModule, *gui);
#else
		world = core::SObject::Create<game::World>(*renderer.get(), *componentModule);
#endif
		gc->SetRootSet(world);

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

			threadSyncManager.Sync();
			gc->Update();
			threadSyncManager.AwakeThread();
		}
	}
}