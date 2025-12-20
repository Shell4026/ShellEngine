#pragma once

#include "Export.h"

#include "External/imgui/imgui.h"
#include "External/imgui/imgui_internal.h"
#include "External/imgui/misc/cpp/imgui_stdlib.h"

#include "Core/ISyncable.h"

#include "Window/Window.h"

#include "Render/IRenderContext.h"

#include <array>

namespace sh::render
{
	class Renderer;
}
namespace sh::game
{
	/// @brief ImGUI의 엔진 구현체 클래스.
	/// 현재 Vulkan만 구현 돼있음.
	class ImGUImpl : public core::ISyncable
	{
	private:
		window::Window& window;
		render::Renderer& renderer;

		ImDrawData drawData;

		bool bInit;
		bool bDirty;
	private:
		void WindowInit();
	protected:
		SH_GAME_API void Sync() override;
	public:
		//TODO: 다른 렌더러 구현
		SH_GAME_API ImGUImpl(window::Window& window, render::Renderer& renderer);
		SH_GAME_API ~ImGUImpl();
		SH_GAME_API void Clean();
		SH_GAME_API void ClearDrawData();

		SH_GAME_API void Init();
		SH_GAME_API void Resize();
		SH_GAME_API void ProcessEvent(window::Event event);
		SH_GAME_API void Begin();
		SH_GAME_API void End();

		SH_GAME_API bool IsInit() const;

		SH_GAME_API auto GetContext() const -> ImGuiContext*;

		/// @brief [렌더 스레드 전용]
		SH_GAME_API auto GetDrawData() -> ImDrawData&;

		SH_GAME_API void SyncDirty() override;
	};
}