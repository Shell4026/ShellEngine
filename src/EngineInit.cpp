#include "EngineInit.h"

#include "Core/Logger.h"
#include "Core/GarbageCollection.h"
#include "Core/ThreadSyncManager.h"
#include "Core/ThreadPool.h"
#include "Core/Factory.hpp"

#include "Window/Window.h"
#include "Render/VulkanImpl/VulkanRenderer.h"

#include "Game/ImGUImpl.h"
#include "Game/RenderThread.h"
#include "Game/Input.h"
#include "Game/Component/Component.h"
#include "Game/ComponentModule.h"
#include "Game/GameManager.h"

#if SH_EDITOR
#include "Editor/EditorResource.h"
#include "Editor/EditorWorld.h"
#include "Editor/AssetDatabase.h"
#else
#include "Core/AssetBundle.h"
#include "Core/AssetResolver.h"

#include "Render/VulkanImpl/VulkanShaderPassBuilder.h"
#include "Render/VulkanImpl/VulkanContext.h"

#include "Game/World.h"
#include "Game/AssetLoaderFactory.h"
#include "Game/Asset/TextureLoader.h"
#include "Game/Asset/ModelLoader.h"
#include "Game/Asset/MeshLoader.h"
#include "Game/Asset/MaterialLoader.h"
#include "Game/Asset/ShaderLoader.h"
#include "Game/Asset/WorldLoader.h"
#include "Game/Asset/PrefabLoader.h"
#include "Game/Asset/TextLoader.h"
#include "Game/Asset/FontLoader.h"

#include "Game/Asset/TextureAsset.h"
#include "Game/Asset/MaterialAsset.h"
#include "Game/Asset/ShaderAsset.h"
#include "Game/Asset/ModelAsset.h"
#include "Game/Asset/MeshAsset.h"
#include "Game/Asset/WorldAsset.h"
#include "Game/Asset/PrefabAsset.h"
#include "Game/Asset/TextAsset.h"
#include "Game/Asset/FontAsset.h"
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
		editor::EditorResource::Destroy();
#endif
		gameManager->Destroy();
		renderer->WaitForCurrentFrame();

		while(gc->GetRootSetCount() != gc->GetObjectCount())
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
				SH_INFO("FocusIn");
				break;
			case sh::window::Event::EventType::WindowFocusOut:
				SH_INFO("FocusOut");
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

		SH_INFO("UIContext initialization");
		gui = std::make_unique<game::ImGUImpl>(*window, static_cast<render::vk::VulkanRenderer&>(*renderer));
		gui->Init();

		SH_INFO("GameManager initialization");
		gameManager = game::GameManager::GetInstance();
		gameManager->Init(*renderer, *gui);

		auto& worldFactory = *core::Factory<game::World, game::World*>::GetInstance();
		worldFactory.Register(game::World::GetStaticType().name.ToString(),
			[renderer = renderer.get(), componentModule, gui = gui.get()]() -> game::World*
			{
				return core::SObject::Create<game::World>(*renderer, *gui);
			}
		);

		SH_INFO("Thread creation");
		core::ThreadPool::GetInstance()->Init(std::max(2u, std::thread::hardware_concurrency() / 2));
		core::ThreadSyncManager::Init();
#if SH_EDITOR
		game::Component::SetEditor(true);

		project = std::make_unique<editor::Project>(*renderer, *gui);

		worldFactory.Register(editor::EditorWorld::GetStaticType().name.ToString(),
			[project = project.get()]() -> game::World*
			{
				return core::SObject::Create<editor::EditorWorld>(*project);
			}
		);
		auto defaultWorld = core::SObject::Create<editor::EditorWorld>(*project); // 기본 월드
		gameManager->LoadWorld(defaultWorld->GetUUID());
#else
		static render::vk::VulkanShaderPassBuilder vkShaderPassBuilder{ static_cast<render::vk::VulkanContext&>(*renderer->GetContext()) };

		auto assetLoaderFactory = game::AssetLoaderFactory::GetInstance();
		assetLoaderFactory->RegisterLoader(game::TextureAsset::ASSET_NAME, std::make_unique<game::TextureLoader>(*renderer->GetContext()));
		assetLoaderFactory->RegisterLoader(game::ModelAsset::ASSET_NAME, std::make_unique<game::ModelLoader>(*renderer->GetContext()));
		assetLoaderFactory->RegisterLoader(game::MeshAsset::ASSET_NAME, std::make_unique<game::MeshLoader>(*renderer->GetContext()));
		assetLoaderFactory->RegisterLoader(game::MaterialAsset::ASSET_NAME, std::make_unique<game::MaterialLoader>(*renderer->GetContext()));
		assetLoaderFactory->RegisterLoader(game::ShaderAsset::ASSET_NAME, std::make_unique<game::ShaderLoader>(&vkShaderPassBuilder));
		assetLoaderFactory->RegisterLoader(game::WorldAsset::ASSET_NAME, std::make_unique<game::WorldLoader>(*renderer, *gui));
		assetLoaderFactory->RegisterLoader(game::PrefabAsset::ASSET_NAME, std::make_unique<game::PrefabLoader>());
		assetLoaderFactory->RegisterLoader(game::TextAsset::ASSET_NAME, std::make_unique<game::TextLoader>());
		assetLoaderFactory->RegisterLoader(game::FontAsset::ASSET_NAME, std::make_unique<game::FontLoader>());

		core::AssetResolverRegistry::SetResolver(
			[this](const core::UUID& uuid) -> core::SObject*
			{
				auto asset = assetBundle->LoadAsset(uuid);
				if (asset == nullptr)
					return nullptr;
				auto assetLoader = game::AssetLoaderFactory::GetInstance()->GetLoader(asset->GetType());
				if (assetLoader == nullptr)
					return nullptr;
				core::SObject* assetPtr = assetLoader->Load(*asset);
				return assetPtr;
			}
		);

		assetBundle = std::make_unique<core::AssetBundle>();
		if (!assetBundle->LoadBundle("assets.bundle"))
			return;
		if (!gameManager->LoadGame("gameManager.bin", *assetBundle))
			return;
 #endif
		SH_INFO("Render thread creation");
		renderThread = game::RenderThread::GetInstance();
		renderThread->Init(*renderer);
		core::ThreadSyncManager::AddThread(*renderThread);

		InitResource();

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

			gameManager->UpdateWorlds(window->GetDeltaTime()); // 스레드간 Sync도 여기서 이뤄짐
			gc->Update();

			core::ThreadSyncManager::AwakeThread();
		}
	}
}
