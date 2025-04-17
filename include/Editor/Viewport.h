#pragma once

#include "Export.h"
#include "UI.h"

#include "Core/ISyncable.h"
#include "Core/Observer.hpp"

#include "Render/VulkanImpl/VulkanConfig.h"
#include "Render/RenderTexture.h"

#include "Game/Component/PickingCamera.h"

#include <mutex>
#include <array>
#include <memory>

namespace sh::game
{
	class ImGUImpl;
	class EditorCamera;
}

namespace sh::editor
{
	class EditorWorld;

	class Viewport : 
		public UI, 
		public core::ISyncable
	{
	private:
		static constexpr int GAME_THREAD = 0;
		static constexpr int RENDER_THREAD = 1;

		float x, y;
		float viewportWidthLast;
		float viewportHeightLast;
		glm::vec2 mousePos;

		render::RenderTexture* renderTex;
		render::RenderTexture* outlineTex = nullptr;
		core::SyncArray<VkDescriptorSet> viewportDescSet;
		
		game::EditorCamera* editorCamera = nullptr;
		game::PickingCamera* pickingCamera = nullptr;

		core::Observer<true,game::PickingCamera::PixelData>::Listener pickingListener{};

		bool bDirty;
		bool bMouseDown;
		bool bFocus;
		bool bOverlay = true;
	protected:
		EditorWorld& world;
	public:
		static constexpr const char* name = "Viewport";
	private:
		/// @brief 뷰포트 사이즈가 변했을 시 렌더 텍스쳐의 사이즈를 바꾸는 함수.
		void ChangeViewportSize();
		void RenderOverlay();
	public:
		SH_EDITOR_API Viewport(game::ImGUImpl& imgui, EditorWorld& world);
		SH_EDITOR_API ~Viewport() noexcept;

		SH_EDITOR_API void Update() override;
		SH_EDITOR_API void Render() override;

		/// @brief 렌더텍스쳐를 가져오는 함수.
		/// @return 렌더 텍스쳐
		SH_EDITOR_API auto GetRenderTexture() -> render::RenderTexture&;

		SH_EDITOR_API void Clean();

		SH_EDITOR_API void SyncDirty() override;
		SH_EDITOR_API void Sync() override;
	};
}//namespace