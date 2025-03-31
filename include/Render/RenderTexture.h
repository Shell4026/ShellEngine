#pragma once

#include "Export.h"
#include "Texture.h"

#include "glm/vec2.hpp"

namespace sh::render
{
	class IRenderContext;
	class Framebuffer;

	class RenderTexture : public Texture
	{
		SCLASS(RenderTexture);
	private:
		PROPERTY(width);
		PROPERTY(height);

		uint32_t width;
		uint32_t height;

		core::SyncArray<std::unique_ptr<Framebuffer>> framebuffer;

		bool bChangeSize = false;
		bool bReadUsage = false;
	public:
		mutable core::Observer<false, const RenderTexture*> onResize;
	private:
		void Resize(uint32_t width, uint32_t height);
	public:
		SH_RENDER_API RenderTexture(Texture::TextureFormat format = Texture::TextureFormat::SRGBA32);
		SH_RENDER_API RenderTexture(RenderTexture&& other) noexcept;
		SH_RENDER_API ~RenderTexture();
		
		/// @brief CPU에서 텍스쳐를 읽을 목적으로 쓰는지 정하는 함수
		/// @param bReadUsage true면 CPU에서 읽을 수 있게 한다.
		SH_RENDER_API void SetReadUsage(bool bReadUsage) noexcept;
		SH_RENDER_API void Build(const IRenderContext& context) override;

		/// @brief 프레임 버퍼를 가져오는 함수
		/// @param thr 호출 하는 스레드
		/// @return 프레임버퍼 포인터
		SH_RENDER_API auto GetFramebuffer(core::ThreadType thr) const -> Framebuffer*;
		SH_RENDER_API auto GetPixelData() const -> const std::vector<Byte>& override;
		SH_RENDER_API void SetSize(uint32_t width, uint32_t height);
		SH_RENDER_API auto GetSize() const -> glm::vec2;

		SH_RENDER_API void Sync() override;
#if SH_EDITOR
		SH_RENDER_API void OnPropertyChanged(const core::reflection::Property& property) override;
#endif
	};
}