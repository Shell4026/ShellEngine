#pragma once
#include "../Export.h"
#include "VulkanConfig.h"
#include "../IShaderBinding.h"

#include <vector>
namespace sh::render::vk
{
	class VulkanContext;

	/// @brief 디스크립터 셋을 추상화한 클래스
	class VulkanDescriptorSet : public IShaderBinding
	{
	public:
		SH_RENDER_API VulkanDescriptorSet();
		SH_RENDER_API VulkanDescriptorSet(VulkanDescriptorSet&& other) noexcept;
		SH_RENDER_API ~VulkanDescriptorSet();

		SH_RENDER_API void Create(const IRenderContext& context, const ShaderPass& shader, UniformStructLayout::Usage usage) override;
		SH_RENDER_API void Create(const IRenderContext& context, const ComputeShader& shader) override;
		SH_RENDER_API void Clear() override;
		SH_RENDER_API void Link(uint32_t binding, const IBuffer& buffer, std::size_t bufferSize) override;
		SH_RENDER_API void Link(uint32_t binding, const Texture& texture) override;

		SH_RENDER_API auto GetSetIndex() const -> uint32_t { return set; }
		SH_RENDER_API auto GetVkDescriptorSet() const -> VkDescriptorSet { return descSet; }
	private:
		const VulkanContext* context;

		VkDescriptorSet descSet;
		uint32_t set = 0;

		std::map<uint32_t, VkDescriptorType> descriptorTypes;
	};
}//namespace