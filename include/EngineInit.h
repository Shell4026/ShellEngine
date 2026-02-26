#pragma once
#include "Core/ModuleLoader.h"

#if SH_EDITOR
#include "Editor/Project.h"
#endif
#include <memory>
namespace sh
{
	namespace core
	{
		class GarbageCollection;
		class AssetBundle;
	}
	namespace window
	{
		class Window;
	}
	namespace render
	{
		class Renderer;
	}
	namespace game
	{
		class ImGUImpl;
		class ComponentModule;
		class GameThread;
		class RenderThread;
		class GameManager;
	}

	class EngineInit
	{
	public:
		EngineInit();
		~EngineInit();

		void Start();
		void Clean();
	private:
		void LoadModule();
		void InitResource();
		void ProcessInput();
		void Loop();
		void SyncThread();
	private:
		core::ModuleLoader moduleLoader;

		core::GarbageCollection* gc = nullptr;
		game::GameThread* gameThread = nullptr;
		game::RenderThread* renderThread = nullptr;
		game::ComponentModule* componentModule = nullptr;
		game::GameManager* gameManager = nullptr;

		std::unique_ptr<window::Window> window;
		std::unique_ptr<game::ImGUImpl> gui;
#if SH_EDITOR
		std::unique_ptr<editor::Project> project;
#else
		std::unique_ptr<core::AssetBundle> assetBundle;
#endif
		std::unique_ptr<render::Renderer> renderer;

		int limitFps = 144;

		bool bStop = false;
	};
}