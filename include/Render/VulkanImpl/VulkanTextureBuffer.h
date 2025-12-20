//#pragma once
//#include "../Export.h"
//#include "../ITextureBuffer.h"
//
//#include "VulkanImageBuffer.h"
//
//#include <memory>
//
//namespace sh::render::vk
//{
//	class VulkanContext;
//	class VulkanQueueManager;
//
//	class VulkanTextureBuffer : public ITextureBuffer
//	{
//	public:
//		SH_RENDER_API VulkanTextureBuffer();
//		SH_RENDER_API VulkanTextureBuffer(VulkanTextureBuffer&& other) noexcept;
//		SH_RENDER_API ~VulkanTextureBuffer();
//
//		SH_RENDER_API void Create(const IRenderContext& context, const CreateInfo& info) override;
//		SH_RENDER_API void Clean() override;
//
//		/// @brief [게임 스레드용] 텍스쳐 버퍼에 데이터를 지정한다.
//		/// @param data 데이터
//		/// @param mipLevel 밉 레벨
//		SH_RENDER_API void SetData(const void* data, uint32_t mipLevel = 1) override;
//
//		SH_RENDER_API auto GetImageBuffer() const -> VulkanImageBuffer*;
//		SH_RENDER_API auto GetSize() const -> std::size_t;
//	private:
//		void CopyBufferToImageCommand(VkCommandBuffer cmd, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t mipLevel);
//	private:
//		const VulkanContext* context = nullptr;
//		VulkanQueueManager* queueManager = nullptr;
//
//		std::unique_ptr<VulkanImageBuffer> imgBuffer;
//
//		uint32_t width = 32, height = 32, channel = 4;
//
//		VkFormat format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
//
//		bool isRenderTexture;
//	};
//}