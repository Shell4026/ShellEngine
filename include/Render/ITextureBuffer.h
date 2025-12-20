#pragma once
#include "Export.h"
#include "Formats.hpp"

#include <cstdint>
#include <variant>
namespace sh::render
{
	class IRenderContext;

	class ITextureBuffer
	{
	public:
		struct CreateInfo
		{
			TextureFormat format;
			uint32_t width;
			uint32_t height;
			uint32_t aniso = 0;
			uint32_t filtering = 1;
			uint32_t mipLevel = 1;
			bool bRenderTarget = false;
			bool bMSAAImg = false;
		};
	public:
		SH_RENDER_API virtual ~ITextureBuffer() = default;
		SH_RENDER_API virtual auto Create(const IRenderContext& context, const CreateInfo& info) -> bool = 0;
		SH_RENDER_API virtual void Clear() = 0;

		SH_RENDER_API virtual void SetData(const void* data, uint32_t mipLevel) = 0;
	};
}