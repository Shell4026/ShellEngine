#pragma once

#include "Export.h"
#include "UI.h"

#include "Core/ISyncable.h"

#include "Render/VulkanImpl/VulkanConfig.h"
#include "Render/RenderTexture.h"

#include <mutex>
#include <array>
#include <memory>

namespace sh::game
{
	class ImGUI;
	class World;
}

namespace sh::editor
{
	class Viewport : 
		public UI, 
		public core::ISyncable
	{
	private:
		static constexpr int GAME_THREAD = 0;
		static constexpr int RENDER_THREAD = 1;

		float viewportWidthLast;
		float viewportHeightLast;

		std::unique_ptr<render::RenderTexture> renderTex;
		core::SyncArray<VkDescriptorSet> viewportDescSet;
		
		bool bDirty;
	protected:
		game::World& world;

		std::mutex& renderMutex;
	public:
		static constexpr char* name = "Viewport";
	private:
		/// @brief 뷰포트 사이즈가 변했을 시 렌더 텍스쳐의 사이즈를 바꾸는 함수.
		void ChangeViewportSize();
	public:
		SH_EDITOR_API Viewport(game::ImGUI& imgui, game::World& world, std::mutex& renderMutex);
		SH_EDITOR_API ~Viewport() noexcept;

		SH_EDITOR_API void Update();
		SH_EDITOR_API void Render();

		/// @brief 렌더텍스쳐를 가져오는 함수.
		/// @return 렌더 텍스쳐
		SH_EDITOR_API auto GetRenderTexture() -> render::RenderTexture&;

		SH_EDITOR_API void Clean();

		SH_EDITOR_API void SetDirty() override;
		SH_EDITOR_API void Sync() override;
	};
}//namespace