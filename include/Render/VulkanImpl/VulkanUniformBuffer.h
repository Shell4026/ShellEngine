#pragma once

#include "Export.h"
#include "IUniformBuffer.h"

namespace sh::render
{
	class VulkanRenderer;

	namespace impl
	{
		/// @brief 디스크립터 셋을 추상화한 클래스
		class VulkanUniformBuffer : public IUniformBuffer
		{
		private:
			const VulkanRenderer* renderer;
			
			VkDescriptorSet descSet;
		public:
			SH_RENDER_API VulkanUniformBuffer();
			SH_RENDER_API VulkanUniformBuffer(VulkanUniformBuffer&& other) noexcept;
			SH_RENDER_API ~VulkanUniformBuffer();

			SH_RENDER_API void Create(const Renderer& renderer, const Shader& shader, uint32_t type) override;
			SH_RENDER_API void Clean() override;
			SH_RENDER_API void Update(uint32_t binding, const IBuffer& buffer) override;
			SH_RENDER_API void Update(uint32_t binding, const Texture& texture) override;

			SH_RENDER_API auto GetVkDescriptorSet() const -> VkDescriptorSet;
		};
	}
}//namespace