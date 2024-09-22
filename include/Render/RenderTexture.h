#pragma once

#include "Export.h"
#include "Texture.h"

#include "glm/vec2.hpp"

namespace sh::render
{
	class Renderer;
	class Framebuffer;
	class RenderTexture : public Texture
	{
		SCLASS(RenderTexture);
	private:
		PROPERTY(width);
		uint32_t width;
		PROPERTY(height);
		uint32_t height;

		core::SyncArray<std::unique_ptr<Framebuffer>> framebuffer;
	public:
		SH_RENDER_API RenderTexture();
		SH_RENDER_API RenderTexture(RenderTexture&& other) noexcept;
		SH_RENDER_API ~RenderTexture();

		SH_RENDER_API void Build(Renderer& renderer) override;

		/// @brief [렌더 스레드용] 프레임 버퍼를 가져온다.
		/// @return 프레임버퍼 포인터
		SH_RENDER_API auto GetFramebuffer() const -> Framebuffer*;

		SH_RENDER_API void SetSize(uint32_t width, uint32_t height);
		SH_RENDER_API auto GetSize() const -> glm::vec2;

		SH_RENDER_API void Sync() override;
#if SH_EDITOR
		SH_RENDER_API void OnPropertyChanged(const core::reflection::Property& property) override;
#endif
	};
}