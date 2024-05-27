#pragma once

#include "Export.h"

#include "Core/NonCopyable.h"
#include "Core/SObject.h"

#include <cstdint>
#include <vector>
#include <memory>

namespace sh::render
{
	class ITextureBuffer;
	class VulkanRenderer;

	class Texture : public core::SObject, public core::INonCopyable
	{
	public:
		enum class TextureFormat
		{
			RGB24,
			RGBA32
		};

		using Byte = unsigned char;
	private:
		std::vector<Byte> pixels;

		std::unique_ptr<ITextureBuffer> buffer;
	public:
		const uint32_t width;
		const uint32_t height;
		const TextureFormat format;
	public:
		SH_RENDER_API Texture(TextureFormat format, uint32_t width, uint32_t height);
		SH_RENDER_API Texture(Texture&& other) noexcept;
		SH_RENDER_API ~Texture();

		SH_RENDER_API void SetPixelData(void* data);
		SH_RENDER_API auto GetPixelData() const -> const std::vector<Byte>&;

		SH_RENDER_API void Build(const VulkanRenderer& renderer);

		SH_RENDER_API auto GetBuffer() -> ITextureBuffer*;
	};
}