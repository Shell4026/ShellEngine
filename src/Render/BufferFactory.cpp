#include "BufferFactory.h"
#include "Renderer.h"

#include "VulkanContext.h"
#include "VulkanImpl/VulkanBuffer.h"
#include "VulkanImpl/VulkanDescriptorSet.h"

#include "Core/Logger.h"

#include <exception>

namespace sh::render
{
	auto BufferFactory::CreateVkUniformBuffer(const vk::VulkanContext& context, const CreateInfo& info) -> std::unique_ptr<IBuffer>
	{
		std::unique_ptr<vk::VulkanBuffer> buffer = std::make_unique<vk::VulkanBuffer>(context);

		VkBufferUsageFlags usage = 
			info.bDynamic ? VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

		usage |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		usage |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		const VkMemoryPropertyFlags memFlag = info.bGPUOnly ? VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT :
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		const VkResult result = buffer->Create(info.size, usage, VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,memFlag, true);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
		{
			SH_ERROR_FORMAT("Can't create buffer: {}", string_VkResult(result));
			throw std::runtime_error{ "Buffer creation failed" };
		}
		return buffer;
	}

	auto BufferFactory::Create(const IRenderContext& context, const CreateInfo& info) -> std::unique_ptr<IBuffer>
	{
		assert(context.GetRenderAPIType() == RenderAPI::Vulkan);
		if (context.GetRenderAPIType() == RenderAPI::Vulkan)
			return CreateVkUniformBuffer(static_cast<const vk::VulkanContext&>(context), info);

		return nullptr;
	}
	auto BufferFactory::CreateShaderBinding(const IRenderContext& context, const ShaderPass& shader, UniformStructLayout::Usage usage) -> std::unique_ptr<IShaderBinding>
	{
		if (context.GetRenderAPIType() == RenderAPI::Vulkan)
		{
			auto ptr = std::make_unique<vk::VulkanDescriptorSet>();
			ptr->Create(context, shader, usage);
			return ptr;
		}
		else
			assert(false);
		return nullptr;
	}
}//namespace