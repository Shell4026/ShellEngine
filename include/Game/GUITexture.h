#pragma once
#include "Export.h"
#include "ImGUImpl.h"

#include "Core/SObject.h"
#include "Core/NonCopyable.h"
#include "Core/Observer.hpp"
#include "Core/SContainer.hpp"
#include "Core/ISyncable.h"

#include "Render/Texture.h"

namespace sh::game
{
	/// @brief ImGUI에서 쓰는 텍스쳐
	class GUITexture : public core::SObject, public core::ISyncable, core::INonCopyable
	{
		SCLASS(GUITexture)
	public:
		SH_GAME_API GUITexture();
		SH_GAME_API GUITexture(GUITexture&& other) noexcept;
		SH_GAME_API ~GUITexture();

		SH_GAME_API void OnDestroy() override;
		SH_GAME_API void Destroy() override;
		SH_GAME_API void Create(const render::Texture& texture);
		SH_GAME_API void Clean();
		SH_GAME_API auto IsValid() const -> bool;

		/// @brief ImGui::Image()와 같다. ImGUI함수 호출중에 호출할 것
		SH_GAME_API void Draw(ImVec2 size, 
			const ImVec2& uv0 = ImVec2(0, 0), 
			const ImVec2& uv1 = ImVec2(1, 1), 
			const ImVec4& tintCol = ImVec4(1, 1, 1, 1), 
			const ImVec4& borderCol = ImVec4(0, 0, 0, 0));
		/// @brief ImGui::ImageButton()와 같다. ImGUI함수 호출중에 호출할 것
		SH_GAME_API void DrawButton(const char* strId, 
			const ImVec2& size,
			const ImVec2& uv0 = ImVec2(0, 0), 
			const ImVec2& uv1 = ImVec2(1, 1), 
			const ImVec4& bgCol = ImVec4(0, 0, 0, 0), 
			const ImVec4& tintCol = ImVec4(1, 1, 1, 1));
	protected:
		SH_GAME_API void Sync() override;
		SH_GAME_API void SyncDirty() override;
	private:
		void InitListeners();
		void UpdateDrawList();
	private:
		const render::IRenderContext* context = nullptr;

		core::Observer<false, const render::Texture*>::Listener onBufferUpdateListener;
		core::Observer<false, const core::SObject*>::Listener onDestroyListener;

		ImTextureID tex = nullptr;

		PROPERTY(originalTex)
		const render::Texture* originalTex = nullptr;

		ImDrawList* drawList = nullptr;

		bool bRecreate = false;
		bool bDirty = false;
	};
}//namespace