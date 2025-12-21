#pragma once
#include "Export.h"
#include "Texture.h"
#include "RenderTarget.h"
#include "RenderData.h"
#include "IRenderThrMethod.h"

#include "glm/vec2.hpp"

namespace sh::render
{
	class IRenderContext;
	class Framebuffer;

	template<>
	struct IRenderThrMethod<class RenderTexture>
	{
		static void ChangeUsage(RenderTexture& rt, ImageUsage newUsage);
	};

	class RenderTexture : public Texture
	{
		friend IRenderThrMethod<RenderTexture>;
		SCLASS(RenderTexture);
	public:
		SH_RENDER_API RenderTexture(const RenderTargetLayout& layout);
		SH_RENDER_API RenderTexture(RenderTexture&& other) noexcept;
		SH_RENDER_API ~RenderTexture();

		SH_RENDER_API void Build(const IRenderContext& context) override;
		SH_RENDER_API void Sync() override;
		SH_RENDER_API void OnPropertyChanged(const core::reflection::Property& property) override;
		
		SH_RENDER_API void SetSize(uint32_t width, uint32_t height);
		SH_RENDER_API auto GetSize() const -> glm::vec2;

		SH_RENDER_API auto GetLayout() const -> const RenderTargetLayout& { return layout; }
		SH_RENDER_API auto GetMSAABuffer() const -> ITextureBuffer* { return msaaBuffer.get(); }
		SH_RENDER_API auto GetDepthBuffer() const -> ITextureBuffer* { return depthBuffer.get(); }
		SH_RENDER_API auto GetUsage() const -> ImageUsage { return usage; }
	protected:
		SH_RENDER_API void ChangeUsage(ImageUsage newUsage);
	private:
		void CreateBuffers();
	private:
		PROPERTY(width);
		uint32_t width;
		PROPERTY(height);
		uint32_t height;

		RenderTargetLayout layout;

		std::unique_ptr<ITextureBuffer> msaaBuffer;
		std::unique_ptr<ITextureBuffer> depthBuffer;

		ImageUsage usage = ImageUsage::Undefined;

		bool bChangeSize = false;
	};

	inline void IRenderThrMethod<class RenderTexture>::ChangeUsage(RenderTexture& rt, ImageUsage newUsage)
	{
		rt.ChangeUsage(newUsage);
	}
}//namespace