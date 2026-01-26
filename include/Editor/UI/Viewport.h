#pragma once
#include "Export.h"

#include "Core/ISyncable.h"
#include "Core/Observer.hpp"

#include "Render/VulkanImpl/VulkanConfig.h"
#include "Render/RenderTexture.h"

#include "Game/Component/Render/PickingCamera.h"

#include "imgui.h"

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

	class Viewport : public core::ISyncable
	{
	public:
		SH_EDITOR_API Viewport(EditorWorld& world);
		SH_EDITOR_API ~Viewport() noexcept;

		SH_EDITOR_API void BlockLeftClick(bool bBlock);
		SH_EDITOR_API void BlockRightClick(bool bBlock);

		SH_EDITOR_API void Update();
		SH_EDITOR_API void Render();

		/// @brief 렌더텍스쳐를 가져오는 함수.
		/// @return 렌더 텍스쳐
		SH_EDITOR_API auto GetRenderTexture() -> render::RenderTexture&;

		SH_EDITOR_API void Clean();

		SH_EDITOR_API void SyncDirty() override;

		SH_EDITOR_API auto Play(bool bStartWorld = true) -> bool;
		SH_EDITOR_API void Stop();

		SH_EDITOR_API auto IsPlaying() const -> bool;
	protected:
		SH_EDITOR_API void Sync() override;
	private:
		/// @brief 뷰포트 사이즈가 변했을 시 렌더 텍스쳐의 사이즈를 바꾸는 함수.
		void ChangeViewportSize();
		void RenderOverlay();
		void RenderPopup();
		void LeftClick();
		void RightClick();
	public:
		static constexpr const char* name = "Viewport";
	protected:
		EditorWorld& world;
	private:
		float x, y;
		float viewportWidthLast;
		float viewportHeightLast;
		glm::vec2 mousePos;

		render::RenderTexture* renderTex;
		render::RenderTexture* outlineTex = nullptr;

		ImTextureID viewportTexture = nullptr;

		game::EditorCamera* editorCamera = nullptr;
		game::PickingCamera* pickingCamera = nullptr;

		core::Observer<true, game::PickingCamera::PixelData>::Listener pickingListener{};

		ImDrawList* imguiDrawList = nullptr;

		bool bDirty;
		bool bMouseLeftDown;
		bool bMouseRightDown;
		bool bFocus;
		bool bOverlay = true;
		bool bBlockRightClick = false;
		bool bBlockLeftClick = false;
		bool bPlaying = false;
	};
}//namespace