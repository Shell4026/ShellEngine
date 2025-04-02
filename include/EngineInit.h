#pragma once

#include "Core/ModuleLoader.h"

#include <memory>

namespace sh
{
	namespace core
	{
		class GarbageCollection;
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
		class World;
		class ImGUImpl;
		class ComponentModule;
		class GameThread;
		class RenderThread;
	}

	class EngineInit
	{
	private:
		core::ModuleLoader moduleLoader;

		core::GarbageCollection* gc = nullptr;
		game::GameThread* gameThread = nullptr;
		game::RenderThread* renderThread = nullptr;
		game::ComponentModule* componentModule = nullptr;

		std::unique_ptr<window::Window> window;
		std::unique_ptr<game::ImGUImpl> gui;

		std::unique_ptr<render::Renderer> renderer;
		game::World* world = nullptr;

		int limitFps = 144;

		bool bStop = false;
	private:
		inline void LoadModule();
		inline void InitResource();
		inline void ProcessInput();
		inline void Loop();
		inline void SyncThread();
	public:
		EngineInit();
		~EngineInit();

		void Start();
		void Clean();
	};
}