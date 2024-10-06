#pragma once

#include "Export.h"

#include "External/imgui/imgui.h"
#include "External/imgui/imgui_internal.h"
#include "External/imgui/backends/imgui_impl_vulkan.h"

#include "Core/ISyncable.h"

#include "Window/Window.h"

#include "Render/VulkanImpl/VulkanRenderer.h"

#include <array>

namespace sh::game
{
	/// @brief ImGUI의 엔진 구현체 클래스.
	/// 현재 Vulkan만 구현 돼있음.
	class ImGUImpl : public core::ISyncable
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
		SH_GAME_API ImGUImpl(window::Window& window, render::VulkanRenderer& renderer);
		SH_GAME_API ~ImGUImpl();
		SH_GAME_API void Clean();

		/// @brief ImGUI세팅 후 렌더러에 드로우콜을 추가하는 함수.
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