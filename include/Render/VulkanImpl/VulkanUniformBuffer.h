#pragma once

#include "Export.h"
#include "IUniformBuffer.h"

namespace sh::render::vk
{
	class VulkanContext;

	/// @brief 디스크립터 셋을 추상화한 클래스
	class VulkanUniformBuffer : public IUniformBuffer
	{
	private:
		const VulkanContext* context;

		VkDescriptorSet descSet;
	public:
		SH_RENDER_API VulkanUniformBuffer();
		SH_RENDER_API VulkanUniformBuffer(VulkanUniformBuffer&& other) noexcept;
		SH_RENDER_API ~VulkanUniformBuffer();

		SH_RENDER_API void Create(const IRenderContext& context, const ShaderPass& shader, uint32_t type) override;
		SH_RENDER_API void Clean() override;
		SH_RENDER_API void Update(uint32_t binding, const IBuffer& buffer) override;
		SH_RENDER_API void Update(uint32_t binding, const Texture& texture) override;

		SH_RENDER_API auto GetVkDescriptorSet() const -> VkDescriptorSet;
	};
}//namespace