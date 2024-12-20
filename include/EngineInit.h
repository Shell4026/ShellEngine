﻿#pragma once

#include "Core/ModuleLoader.h"
#include "Core/ThreadSyncManager.h"

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
		core::ThreadSyncManager threadSyncManager;

		core::GarbageCollection* gc;
		game::GameThread* gameThread;
		game::RenderThread* renderThread;
		game::ComponentModule* componentModule;

		std::unique_ptr<window::Window> window;
		std::unique_ptr<game::ImGUImpl> gui;

		render::Renderer* renderer;
		game::World* world;

		int limitFps;

		bool bStop;
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