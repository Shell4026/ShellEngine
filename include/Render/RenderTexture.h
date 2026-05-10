#pragma once
#include "Export.h"
#include "Texture.h"
#include "RenderData.h"
#include "IRenderThrMethod.h"
#include "ITextureBuffer.h"

#include "glm/vec2.hpp"

namespace sh::render
{
	class IRenderContext;
	class Framebuffer;

	template<>
	struct IRenderThrMethod<class RenderTexture>
	{
		static void ChangeUsage(RenderTexture& rt, ResourceUsage newUsage);
	};

	class RenderTexture : public Texture
	{
		friend IRenderThrMethod<RenderTexture>;
		SCLASS(RenderTexture);
	public:
		SH_RENDER_API RenderTexture(TextureFormat colorFormat, TextureFormat depthFormat = TextureFormat::D24S8, bool bMSAA = false);
		SH_RENDER_API RenderTexture(RenderTexture&& other) noexcept;
		SH_RENDER_API ~RenderTexture();

		SH_RENDER_API void Build(const IRenderContext& context) override;
		SH_RENDER_API void Sync() override;
		SH_RENDER_API void OnPropertyChanged(const core::reflection::Property& property) override;
		
		SH_RENDER_API void SetSize(uint32_t width, uint32_t height);

		auto GetSize() const -> glm::vec2 { return { width, height }; }
		auto GetDepthFormat() const -> TextureFormat { return depthFormat; }
		auto GetMSAABuffer() const -> ITextureBuffer* { return msaaBuffer.get(); }
		auto GetDepthBuffer() const -> ITextureBuffer* { return IsDepthTexture() ? textureBuffer.get() : depthBuffer.get(); }
		auto IsDepthTexture() const -> bool { return GetTextureFormat() == TextureFormat::None && depthFormat != TextureFormat::None; }
		auto GetUsage() const -> ResourceUsage { return usage; }
	protected:
		void ChangeUsage(ResourceUsage newUsage) { usage = newUsage; }
	private:
		void CreateBuffers();
		void CreateDepthBuffer(const ITextureBuffer::CreateInfo& ci);
	private:
		PROPERTY(width);
		uint32_t width;
		PROPERTY(height);
		uint32_t height;

		TextureFormat depthFormat = TextureFormat::D24S8;

		std::unique_ptr<ITextureBuffer> msaaBuffer;
		std::unique_ptr<ITextureBuffer> depthBuffer;

		ResourceUsage usage = ResourceUsage::Undefined;

		bool bChangeSize = false;
		bool bMSAA = false;
	};

	inline void IRenderThrMethod<class RenderTexture>::ChangeUsage(RenderTexture& rt, ResourceUsage newUsage)
	{
		rt.ChangeUsage(newUsage);
	}
}//namespace
