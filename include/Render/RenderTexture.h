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
		const Renderer* renderer;

		PROPERTY(width);
		uint32_t width;
		PROPERTY(height);
		uint32_t height;

		std::unique_ptr<Framebuffer> framebuffer;
	public:
		SH_RENDER_API RenderTexture();
		SH_RENDER_API RenderTexture(RenderTexture&& other) noexcept;
		SH_RENDER_API ~RenderTexture();

		SH_RENDER_API void Build(const Renderer& renderer) override;

		SH_RENDER_API auto GetFramebuffer() const -> Framebuffer*;
		//auto GetPixelData() const -> const std::vector<Byte>& override;
		SH_RENDER_API void SetSize(uint32_t width, uint32_t height);
		SH_RENDER_API auto GetSize() const -> glm::vec2;
#if SH_EDITOR
		SH_RENDER_API void OnPropertyChanged(const core::reflection::Property& property) override;
#endif
	};
}