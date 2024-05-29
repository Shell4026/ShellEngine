#pragma once

#include "Export.h"

#include "External/imgui/imgui.h"
#include "External/imgui/backends/imgui_impl_vulkan.h"

#include "Window/Window.h"

#include "Render/VulkanRenderer.h"

namespace sh::game
{
	class ImGUI
	{
	private:
		window::Window& window;
		render::VulkanRenderer& renderer;

		static bool bInit;
	private:
		void WindowInit();
	public:
		SH_GAME_API ImGUI(window::Window& window, render::VulkanRenderer& renderer);
		SH_GAME_API ~ImGUI();
		SH_GAME_API void Clean();

		SH_GAME_API void Init();
		SH_GAME_API void ProcessEvent(window::Event event);
		SH_GAME_API void Update();
		SH_GAME_API void Render();

		SH_GAME_API static bool IsInit();
	};
}