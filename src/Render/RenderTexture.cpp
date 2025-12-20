#include "RenderTexture.h"
#include "VulkanImageBuffer.h"
#include "IRenderContext.h"

namespace sh::render
{
	RenderTexture::RenderTexture(const RenderTargetLayout& layout) :
		Texture(layout.format, 1024, 1024, false),

		width(1024), height(1024), 
		layout(layout)
	{
		SetAnisoLevel(0);
	}
	RenderTexture::RenderTexture(RenderTexture&& other) noexcept :
		Texture(std::move(other)),

		width(other.width), height(other.height),
		layout(other.layout)
	{
	}
	RenderTexture::~RenderTexture()
	{
	}

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
	SH_RENDER_API auto RenderTexture::GetSize() const -> glm::vec2
	{
		return { width, height };
	}
	SH_RENDER_API void RenderTexture::ChangeUsage(ImageUsage newUsage)
	{
		usage = newUsage;
	}

	void RenderTexture::CreateBuffers()
	{
		assert(context->GetRenderAPIType() == RenderAPI::Vulkan);
		if (textureBuffer == nullptr)
		{
			if (context->GetRenderAPIType() == RenderAPI::Vulkan)
				textureBuffer = std::make_unique<vk::VulkanImageBuffer>();
		}
		ITextureBuffer::CreateInfo ci{};
		ci.width = width;
		ci.height = height;
		ci.format = GetTextureFormat();
		ci.aniso = 0;
		ci.filtering = 1;
		ci.mipLevel = 1;
		ci.bMSAAImg = false;
		ci.bRenderTarget = true;

		textureBuffer->Create(*context, ci);

		if (layout.depthFormat == TextureFormat::D32S8 || 
			layout.depthFormat == TextureFormat::D24S8 ||
			layout.depthFormat == TextureFormat::D16S8)
		{
			if (depthBuffer == nullptr)
			{
				if (context->GetRenderAPIType() == RenderAPI::Vulkan)
					depthBuffer = std::make_unique<vk::VulkanImageBuffer>();
			}
			ci.format = layout.depthFormat;
			depthBuffer->Create(*context, ci);
		}

		usage = ImageUsage::Undefined;
	}
}//namespace