#include "BufferFactory.h"
#include "Renderer.h"

#include "VulkanContext.h"
#include "VulkanImpl/VulkanBuffer.h"
#include "VulkanImpl/VulkanUniformBuffer.h"

#include "Core/Logger.h"

#include <exception>

namespace sh::render
{
	auto BufferFactory::CreateVkUniformBuffer(const vk::VulkanContext& context, std::size_t size, bool bTransferDst) -> std::unique_ptr<IBuffer>
	{
		std::unique_ptr<vk::VulkanBuffer> buffer = std::make_unique<vk::VulkanBuffer>(context);

		VkBufferUsageFlags usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		if (bTransferDst)
			usage |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		auto result = buffer->Create(size, usage,
			VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			true);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
		{
			SH_ERROR_FORMAT("Can't create buffer: {}", string_VkResult(result));
			throw std::runtime_error{ "Buffer creation failed" };
		}
		return buffer;
	}

	auto BufferFactory::Create(const IRenderContext& context, std::size_t size, bool bTransferDst) -> std::unique_ptr<IBuffer>
	{
		assert(context.GetRenderAPIType() == RenderAPI::Vulkan);
		if (context.GetRenderAPIType() == RenderAPI::Vulkan)
			return CreateVkUniformBuffer(static_cast<const vk::VulkanContext&>(context), size, bTransferDst);

		return nullptr;
	}
	auto BufferFactory::CreateUniformBuffer(const IRenderContext& context, const ShaderPass& shader, UniformStructLayout::Type type) -> std::unique_ptr<IUniformBuffer>
	{
		if (context.GetRenderAPIType() == RenderAPI::Vulkan)
		{
			auto ptr = std::make_unique<vk::VulkanUniformBuffer>();
			ptr->Create(context, shader, type);
			return ptr;
		}
		else
			assert(false);
		return nullptr;
	}
}//namespace