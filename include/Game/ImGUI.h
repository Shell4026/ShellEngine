#pragma once

#include "Export.h"

#include "External/imgui/imgui.h"
#include "External/imgui/imgui_internal.h"
#include "External/imgui/backends/imgui_impl_vulkan.h"

#include "Core/ISyncable.h"

#include "Window/Window.h"

#include "Render/VulkanRenderer.h"

#include <array>

namespace sh::game
{
	class ImGUI : public core::ISyncable
	{
	private:
		window::Window& window;
		render::VulkanRenderer& renderer;

		ImDrawData drawData;

		bool bInit;
		bool bDirty;
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

		SH_GAME_API void SetDirty() override;
		SH_GAME_API void Sync() override;
	};
}