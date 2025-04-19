﻿#pragma once
#include "Export.h"

#include "Core/ISyncable.h"
#include "Core/NonCopyable.h"
#include "Core/SObject.h"
#include "Core/Observer.hpp"

#include <cstdint>
#include <memory>
#include <vector>
#include <atomic>
namespace sh::render
{
	class ITextureBuffer;
	class IRenderContext;

	class Texture : 
		public core::SObject, 
		public core::INonCopyable,
		public core::ISyncable
	{
		SCLASS(Texture)
	public:
		enum class TextureFormat
		{
			SRGB24,
			SRGBA32,
			R8,
			RGB24,
			RGBA32
		};

		using Byte = unsigned char;
	private:
		TextureFormat format;

		std::atomic_flag bDirty;
		bool bSRGB = false;
		bool bSetDataDirty = false;
	protected:
		const IRenderContext* context;

		std::unique_ptr<ITextureBuffer> textureBuffer;

		std::vector<Byte> pixels;
	public:
		mutable core::Observer<false, const Texture*> onBufferUpdate;

		const uint32_t width;
		const uint32_t height;
	private:
		void CreateTextureBuffer();
		auto CheckSRGB() const -> bool;
	public:
		SH_RENDER_API Texture(TextureFormat format, uint32_t width, uint32_t height);
		SH_RENDER_API Texture(Texture&& other) noexcept;
		SH_RENDER_API virtual ~Texture();

		/// @brief 픽셀 데이터를 지정한다. 
		/// @brief [주의] 동기화 타이밍에 텍스쳐 버퍼가 재설정됨.
		/// @param data 데이터 포인터
		SH_RENDER_API         void SetPixelData(void* data);
		SH_RENDER_API virtual auto GetPixelData() const -> const std::vector<Byte>&;

		SH_RENDER_API virtual void Build(const IRenderContext& context);

		/// @brief 네이티브 텍스쳐 버퍼를 가져온다.
		/// @return 텍스쳐 버퍼 포인터
		SH_RENDER_API auto GetTextureBuffer() const -> ITextureBuffer*;
		SH_RENDER_API auto GetTextureFormat() const -> TextureFormat;

		/// @brief 텍스쳐 포멧을 변경한다.
		/// @brief [주의] 동기화 타이밍에 텍스쳐 버퍼가 재설정됨.
		/// @param target 텍스쳐 포멧
		SH_RENDER_API void ChangeTextureFormat(TextureFormat target);

		SH_RENDER_API auto IsSRGB() const -> bool;

		SH_RENDER_API void SyncDirty() override;
		SH_RENDER_API void Sync() override;
	};
}