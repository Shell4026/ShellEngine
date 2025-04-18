#include "EngineInit.h"

#include "Core/Logger.h"
#include "Core/GarbageCollection.h"
#include "Core/ThreadSyncManager.h"
#include "Core/ThreadPool.h"

#include "Window/Window.h"

#include "Render/RenderPipeline.h"
#include "Render/VulkanImpl/VulkanRenderer.h"
#include "Render/VulkanImpl/VulkanContext.h"

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
		gc->Collect();
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
		world->InitResource();
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
							renderer->GetContext()->SetViewport({ 0, 0.f }, { width, height }); 
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

		SH_INFO("Window initialization");
		window = std::make_unique<window::Window>();
		window->Create(u8"ShellEngine", 1024, 768, sh::window::Window::Style::Resize);
		window->SetFps(limitFps);

		SH_INFO("Renderer initialization");
		renderer = std::make_unique<sh::render::vk::VulkanRenderer>();
		renderer->Init(*window);
		renderer->GetContext()->SetViewport({ 150.f, 0.f }, { window->width - 150.f, window->height - 180 });

		gui = std::make_unique<game::ImGUImpl>(*window, static_cast<render::vk::VulkanRenderer&>(*renderer));
		gui->Init();
#if SH_EDITOR
		world = core::SObject::Create<editor::EditorWorld>(*renderer, *componentModule, *gui);
#else
		world = core::SObject::Create<game::World>(*renderer.get(), *componentModule);
#endif
		gc->SetRootSet(world);

		renderer->AddRenderPipeline<sh::render::RenderPipeline>();

		SH_INFO("Thread creation");
		core::ThreadPool::GetInstance()->Init(std::max(2u, std::thread::hardware_concurrency() / 2));
		core::ThreadSyncManager::Init();
		renderThread = game::RenderThread::GetInstance();
		renderThread->Init(*renderer);
		core::ThreadSyncManager::AddThread(*renderThread);

		InitResource();

		SH_INFO("Start world");
		world->Start();
		renderThread->Run();

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

			core::ThreadSyncManager::Sync();
			gc->Update();
			core::ThreadSyncManager::AwakeThread();
		}
	}
}