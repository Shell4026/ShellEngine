#pragma once
#include "Export.h"
#include "VulkanConfig.h"
#include "../IUniformBuffer.h"

namespace sh::render::vk
{
	class VulkanContext;

	/// @brief 디스크립터 셋을 추상화한 클래스
	class VulkanUniformBuffer : public IUniformBuffer
	{
	private:
		const VulkanContext* context;

		VkDescriptorSet descSet;
		uint32_t set = 0;

		bool bDynamic = false;
	public:
		SH_RENDER_API VulkanUniformBuffer();
		SH_RENDER_API VulkanUniformBuffer(VulkanUniformBuffer&& other) noexcept;
		SH_RENDER_API ~VulkanUniformBuffer();

		SH_RENDER_API void Create(const IRenderContext& context, const ShaderPass& shader, UniformStructLayout::Type type) override;
		SH_RENDER_API void Clear() override;
		SH_RENDER_API void Link(uint32_t binding, const IBuffer& buffer, std::size_t bufferSize) override;
		SH_RENDER_API void Link(uint32_t binding, const Texture& texture) override;

		SH_RENDER_API auto GetSetNumber() const -> uint32_t;
		SH_RENDER_API auto GetVkDescriptorSet() const -> VkDescriptorSet;
	};
}//namespace