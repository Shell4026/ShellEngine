#include "RenderTexture.h"
#include "VulkanImageBuffer.h"
#include "IRenderContext.h"

#include "Core/Logger.h"

namespace sh::render
{
	RenderTexture::RenderTexture(TextureFormat colorFormat, TextureFormat depthFormat, bool bMSAA) :
		Texture(colorFormat, 1024, 1024, false),

		width(1024), height(1024), 
		depthFormat(depthFormat),
		bMSAA(bMSAA)
	{
		SetAnisoLevel(0);
	}
	RenderTexture::RenderTexture(RenderTexture&& other) noexcept :
		Texture(std::move(other)),

		width(other.width), height(other.height),
		depthFormat(other.depthFormat),
		bMSAA(other.bMSAA)
	{
	}
	RenderTexture::~RenderTexture() = default;

	SH_RENDER_API void RenderTexture::Build(const IRenderContext& context)
	{
		if (this->context == nullptr)
		{
			this->context = &context;
			CreateBuffers();
		}
	}
	SH_RENDER_API void RenderTexture::Sync()
	{
		Super::Sync();
		if (bChangeSize)
		{
			CreateBuffers();
			bChangeSize = false;
			onBufferUpdate.Notify(this);
		}
	}
	SH_RENDER_API void RenderTexture::OnPropertyChanged(const core::reflection::Property& property)
	{
		if (context == nullptr)
			return;
		if (property.GetName() == core::Util::ConstexprHash("width") || property.GetName() == core::Util::ConstexprHash("height"))
		{
			Build(*context);
		}
	}
	SH_RENDER_API void RenderTexture::SetSize(uint32_t width, uint32_t height)
	{
		assert(width != 0 && height != 0);
		this->width = width;
		this->height = height;

		bChangeSize = true;
		SyncDirty();
	}
	void RenderTexture::CreateBuffers()
	{
		assert(context->GetRenderAPIType() == RenderAPI::Vulkan);

		ITextureBuffer::CreateInfo ci{};
		ci.width = width;
		ci.height = height;
		ci.format = IsDepthTexture() ? depthFormat : GetTextureFormat();
		ci.aniso = 0;
		ci.filtering = 1;
		ci.mipLevel = 1;
		ci.bMSAAImg = false;
		ci.bRenderTarget = true;

		usage = ResourceUsage::Undefined;

		if (IsDepthTexture())
		{
			CreateDepthBuffer(ci);
			return;
		}

		if (textureBuffer == nullptr)
			if (context->GetRenderAPIType() == RenderAPI::Vulkan)
				textureBuffer = std::make_unique<vk::VulkanImageBuffer>();
		textureBuffer->Create(*context, ci);

		if (bMSAA)
		{
			ci.bMSAAImg = true;
			if (msaaBuffer == nullptr)
			{
				if (context->GetRenderAPIType() == RenderAPI::Vulkan)
					msaaBuffer = std::make_unique<vk::VulkanImageBuffer>();
			}
			msaaBuffer->Create(*context, ci);
		}
		if (depthFormat != TextureFormat::None)
		{
			ci.format = depthFormat;
			CreateDepthBuffer(ci);
		}
	}
	void RenderTexture::CreateDepthBuffer(const ITextureBuffer::CreateInfo& ci)
	{
		const bool bValidDepthFormat =
			depthFormat == TextureFormat::D32 ||
			depthFormat == TextureFormat::D16 ||
			depthFormat == TextureFormat::D32S8 ||
			depthFormat == TextureFormat::D24S8 ||
			depthFormat == TextureFormat::D16S8;
		if (!bValidDepthFormat)
		{
			SH_ERROR_FORMAT("Invalid depth foramt!: {}", TextureFormatToString(ci.format));
			return;
		}
		if (IsDepthTexture())
		{
			if (textureBuffer == nullptr)
				if (context->GetRenderAPIType() == RenderAPI::Vulkan)
					textureBuffer = std::make_unique<vk::VulkanImageBuffer>();
			textureBuffer->Create(*context, ci);
		}
		else
		{
			if (depthBuffer == nullptr)
				if (context->GetRenderAPIType() == RenderAPI::Vulkan)
					depthBuffer = std::make_unique<vk::VulkanImageBuffer>();
			depthBuffer->Create(*context, ci);
		}
	}
}//namespace