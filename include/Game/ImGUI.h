#pragma once

#include "Export.h"

#include "External/imgui/imgui.h"
#include "External/imgui/imgui_internal.h"
#include "External/imgui/backends/imgui_impl_vulkan.h"

#include "Window/Window.h"

#include "Render/VulkanRenderer.h"

#include <array>

namespace sh::game
{
	class ImGUI
	{
	private:
		static constexpr int GAME_THREAD = 0;
		static constexpr int RENDER_THREAD = 1;

		window::Window& window;
		render::VulkanRenderer& renderer;

		ImDrawData drawData;

		bool bInit;
	private:
		void WindowInit();
	public:
		//TODO: 다른 렌더러 구현
		SH_GAME_API ImGUI(window::Window& window, render::VulkanRenderer& renderer);
		SH_GAME_API ~ImGUI();
		SH_GAME_API void Clean();

		SH_GAME_API void Init();
		SH_GAME_API void Resize();
		SH_GAME_API void ProcessEvent(window::Event event);
		SH_GAME_API void Begin();
		SH_GAME_API void End();

		SH_GAME_API bool IsInit() const;

		SH_GAME_API auto GetContext() const -> ImGuiContext*;

		/// @brief [렌더 스레드용] 게임 스레드와 동기화 하는 함수.
		/// @return 
		SH_GAME_API void SyncDrawData();
	};
}