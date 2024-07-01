#pragma once

#include "Export.h"
#include "VulkanImpl/VulkanConfig.h"
#include "VulkanImpl/VulkanBuffer.h"

#include "Core/NonCopyable.h"

#include <vector>

namespace sh::render
{
	class VulkanRenderer;
	class Material;
	class Shader;

	class VulkanUniform : public sh::core::INonCopyable
	{
	private:
		const VulkanRenderer& renderer;

		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorSet descriptorSet;

		impl::VulkanBuffer uniformBuffer;
	private:
		
		
	public:
		VulkanUniform(const VulkanRenderer& renderer);
		VulkanUniform(VulkanUniform&& other) noexcept;
		~VulkanUniform();

		auto Create(uint32_t binding, size_t dataSize) -> VkResult;
		void Clean();
	};
}