#include "EngineInit.h"

#include "Core/Logger.h"
#include "Core/GarbageCollection.h"
#include "Core/ThreadSyncManager.h"

#include "Window/Window.h"

#include "Render/VulkanImpl/VulkanRenderer.h"
#include "Render/VulkanImpl/VulkanContext.h"
#include "Render/VulkanImpl/VulkanShaderPass.h"
#include "Render/VulkanImpl/VulkanShaderPassBuilder.h"
#include "Render/Mesh/Plane.h"
#include "Render/Mesh/Grid.h"

#include "Game/ShaderLoader.h"
#include "Game/TextureLoader.h"
#include "Game/ModelLoader.h"
#include "Game/ImGUImpl.h"
#include "Game/RenderThread.h"
#include "Game/Input.h"

#include "Game/ComponentModule.h"

#if SH_EDITOR
#include "Editor/EditorWorld.h"
#endif

namespace sh
{
	EngineInit::EngineInit() :
		moduleLoader()
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

		render::vk::VulkanShaderPassBuilder shaderBuilder{ static_cast<sh::render::vk::VulkanContext&>(*renderer->GetContext()) };

		ShaderLoader loader{ &shaderBuilder };
		TextureLoader texLoader{ *renderer->GetContext() };
		ModelLoader modelLoader{ *renderer->GetContext() };

		auto defaultShader = world->shaders.AddResource("Default", loader.LoadShader("shaders/default.shader"));
		auto lineShader = world->shaders.AddResource("Line", loader.LoadShader("shaders/line.shader"));
		auto errorShader = world->shaders.AddResource("ErrorShader", loader.LoadShader("shaders/error.shader"));
		auto gridShader = world->shaders.AddResource("GridShader", loader.LoadShader("shaders/grid.shader"));
		auto pickingShader = world->shaders.AddResource("PickingShader", loader.LoadShader("shaders/picking.shader"));
		auto outlineShader = world->shaders.AddResource("OutlineShader", loader.LoadShader("shaders/outline.shader"));

		auto errorMat = world->materials.AddResource("ErrorMaterial", sh::render::Material{ errorShader });
		auto lineMat = world->materials.AddResource("LineMaterial", sh::render::Material{ lineShader });
		auto gridMat = world->materials.AddResource("GridMaterial", sh::render::Material{ gridShader });
		auto pickingMat = world->materials.AddResource("PickingMaterial", sh::render::Material{ pickingShader });

		auto plane = world->meshes.AddResource("PlaneMesh", sh::render::Plane{});
		auto grid = world->meshes.AddResource("GridMesh", sh::render::Grid{});

		auto ObjectUniformType = render::ShaderPass::UniformType::Object;
		auto MaterialUniformType = render::ShaderPass::UniformType::Material;

		errorShader->SetUUID(core::UUID{ "bbc4ef7ec45dce223297a224f8093f0f" });
		errorShader->SetName("ErrorShader");

		defaultShader->SetUUID(core::UUID{ "ad9217609f6c7e0f1163785746cc153e" });
		defaultShader->SetName("DefaultShader");

		errorMat->Build(*renderer->GetContext());
		pickingMat->Build(*renderer->GetContext());

		lineMat->SetVector("test", glm::vec4{ 1.f, 0.f, 0.f, 1.f });
		lineMat->Build(*renderer->GetContext());

		gridMat->SetVector("color", glm::vec4{ 0.6f, 0.6f, 0.8f, 0.2f });
		gridMat->Build(*renderer->GetContext());

		plane->Build(*renderer->GetContext());
		grid->Build(*renderer->GetContext());
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

		gc = core::GarbageCollection::GetInstance(); // GC 초기화
		threadSyncManager = core::ThreadSyncManager::GetInstance(); // 스레드 동기화 매니저 초기화

		SH_INFO("Window initialization");
		window = std::make_unique<window::Window>();
		window->Create(u8"ShellEngine", 1024, 768, sh::window::Window::Style::Resize);
		window->SetFps(limitFps);

		SH_INFO("Renderer initialization");
		renderer = std::make_unique<sh::render::vk::VulkanRenderer>();
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
		threadSyncManager->AddThread(*renderThread);

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

			threadSyncManager->Sync();
			gc->Update();
			threadSyncManager->AwakeThread();
		}
	}
}