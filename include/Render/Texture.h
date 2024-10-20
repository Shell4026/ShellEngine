#pragma once

#include "Export.h"

#include "Core/ISyncable.h"
#include "Core/NonCopyable.h"
#include "Core/SObject.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace sh::render
{
	class ITextureBuffer;
	class Renderer;

	class Texture : 
		public core::SObject, 
		public core::INonCopyable,
		public core::ISyncable
	{
		SCLASS(Texture)
	private:
		bool bDirty;
	public:
		enum class TextureFormat
		{
			SRGB24,
			SRGBA32,
			RGB24,
			RGBA32
		};

		using Byte = unsigned char;
	protected:
		Renderer* renderer;

		core::SyncArray<std::unique_ptr<ITextureBuffer>> buffer;

		std::vector<Byte> pixels;
	public:
		const uint32_t width;
		const uint32_t height;
		const TextureFormat format;
	public:
		SH_RENDER_API Texture(TextureFormat format, uint32_t width, uint32_t height);
		SH_RENDER_API Texture(Texture&& other) noexcept;
		SH_RENDER_API virtual ~Texture();

		SH_RENDER_API void SetPixelData(void* data);
		SH_RENDER_API virtual auto GetPixelData() const -> const std::vector<Byte>&;

		SH_RENDER_API virtual void Build(Renderer& renderer);

		/// @brief 네이티브 텍스쳐 버퍼를 가져온다.
		/// @param thr 현재 스레드 종류
		/// @return 텍스쳐 버퍼 포인터
		SH_RENDER_API auto GetBuffer(core::ThreadType thr = core::ThreadType::Game) const -> ITextureBuffer*;
		
		SH_RENDER_API void SetDirty() override;
		SH_RENDER_API void Sync() override;
	};
}