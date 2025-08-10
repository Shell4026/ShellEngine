#include "EngineInit.h"

#include "Core/Logger.h"
#include "Core/GarbageCollection.h"
#include "Core/ThreadSyncManager.h"
#include "Core/ThreadPool.h"
#include "Core/Factory.hpp"

#include "Window/Window.h"

#include "Render/RenderPipeline.h"
#include "Render/VulkanImpl/VulkanRenderer.h"
#include "Render/VulkanImpl/VulkanContext.h"

#include "Game/ImGUImpl.h"
#include "Game/RenderThread.h"
#include "Game/Input.h"

#include "Game/ComponentModule.h"
#include "Game/GameManager.h"

#if SH_EDITOR
#include "Editor/EditorResource.h"
#include "Editor/EditorWorld.h"
#include "Editor/AssetDatabase.h"
#else
#include "Core/AssetBundle.h"
#include "Game/World.h"
#include "Game/AssetLoaderFactory.h"
#include "Game/TextureLoader.h"
#include "Game/ModelLoader.h"
#include "Game/ModelAsset.h"
#include "Game/MaterialLoader.h"
#include "Game/ShaderLoader.h"
#include "Game/ShaderAsset.h"
#include "Game/WorldLoader.h"
#include "Game/WorldAsset.h"
#include "Render/VulkanImpl/VulkanShaderPassBuilder.h"
#include "Render/VulkanImpl/VulkanContext.h"
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
#if SH_EDITOR
		project.reset();
		editor::AssetDatabase::Destroy();
#endif
		gameManager->Clean();
		gc->DefragmentRootSet();
		while(gc->GetRootSet().size() != gc->GetObjectCount())
		{
			gc->Collect();
			gc->DestroyPendingKillObjs();
		}
		gui.reset();

		renderer.reset();

		window.reset();
	}

	inline void EngineInit::InitResource()
	{
		SH_INFO("Resource initialization");
#if SH_EDITOR
		editor::EditorResource::GetInstance()->LoadAllAssets(*project);
#endif
		gameManager->GetCurrentWorld()->InitResource();
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
		game::ComponentModule* componentModule = game::ComponentModule::GetInstance();
		componentModule->RegisterWaitingComponents();

		SH_INFO_FORMAT("System thread count: {}", std::thread::hardware_concurrency());

		SH_INFO("GarbageCollection initialization");
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

		gameManager = game::GameManager::GetInstance();

		auto& worldFactory = *core::Factory<game::World, game::World*>::GetInstance();
		worldFactory.Register(game::World::GetStaticType().name.ToString(),
			[renderer = renderer.get(), componentModule, gui = gui.get()]() -> game::World*
			{
				return core::SObject::Create<game::World>(*renderer, *gui);
			}
		);
#if SH_EDITOR
		project = std::make_unique<editor::Project>(*renderer, *gui);

		worldFactory.Register(editor::EditorWorld::GetStaticType().name.ToString(),
			[project = project.get()]() -> game::World*
			{
				return core::SObject::Create<editor::EditorWorld>(*project);
			}
		);
		auto defaultWorld = core::SObject::Create<editor::EditorWorld>(*project); // 기본 월드
		gameManager->AddWorld(*defaultWorld);
		gameManager->SetCurrentWorld(*defaultWorld);
#else
		world = core::SObject::Create<game::World>(*renderer.get(), *componentModule, *gui);
		
		static render::vk::VulkanShaderPassBuilder vkShaderPassBuilder{ static_cast<render::vk::VulkanContext&>(*renderer->GetContext()) };

		auto factory = game::AssetLoaderFactory::GetInstance();
		factory->RegisterLoader(game::TextureAsset::ASSET_NAME, std::make_unique<game::TextureLoader>(*renderer->GetContext()));
		factory->RegisterLoader(game::ModelAsset::ASSET_NAME, std::make_unique<game::ModelLoader>(*renderer->GetContext()));
		//factory->RegisterLoader("MATL", std::make_unique<game::MaterialLoader>(*renderer->GetContext()));
		factory->RegisterLoader(game::ShaderAsset::ASSET_NAME, std::make_unique<game::ShaderLoader>(&vkShaderPassBuilder));
		factory->RegisterLoader(game::WorldAsset::ASSET_NAME, std::make_unique<game::WorldLoader>());

#endif
		SH_INFO("Thread creation");
		core::ThreadPool::GetInstance()->Init(std::max(2u, std::thread::hardware_concurrency() / 2));
		core::ThreadSyncManager::Init();
		renderThread = game::RenderThread::GetInstance();
		renderThread->Init(*renderer);
		core::ThreadSyncManager::AddThread(*renderThread);

		InitResource();

		SH_INFO("Start world");
		gameManager->GetCurrentWorld()->Start();
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

			gameManager->UpdateWorld(window->GetDeltaTime());
			gameManager->GetCurrentWorld()->BeforeSync();
			core::ThreadSyncManager::Sync();
			gameManager->GetCurrentWorld()->AfterSync();

			gc->Update();

			core::ThreadSyncManager::AwakeThread();
		}
	}
}
