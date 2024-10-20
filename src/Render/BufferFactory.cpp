#include "PCH.h"
#include "BufferFactory.h"
#include "Renderer.h"

#include "VulkanRenderer.h"
#include "VulkanImpl/VulkanBuffer.h"
#include "VulkanImpl/VulkanUniformBuffer.h"

#include "Core/Logger.h"

#include <exception>

namespace sh::render
{
	auto BufferFactory::CreateVkUniformBuffer(const VulkanRenderer& renderer, std::size_t size, bool bTransferDst) -> std::unique_ptr<IBuffer>
	{
		std::unique_ptr<impl::VulkanBuffer> buffer = std::make_unique<impl::VulkanBuffer>(renderer.GetDevice(), renderer.GetGPU(), renderer.GetAllocator());

		VkBufferUsageFlags usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		if (bTransferDst)
			usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT;

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

	auto BufferFactory::Create(const Renderer& renderer, std::size_t size, bool bTransferDst) -> std::unique_ptr<IBuffer>
	{
		assert(renderer.apiType == RenderAPI::Vulkan);
		if (renderer.apiType == RenderAPI::Vulkan)
		{
			return CreateVkUniformBuffer(static_cast<const VulkanRenderer&>(renderer), size, bTransferDst);
		}
		return nullptr;
	}
	auto BufferFactory::CreateUniformBuffer(const Renderer& renderer, const Shader& shader, Shader::UniformType type) -> std::unique_ptr<IUniformBuffer>
	{
		if (renderer.apiType == RenderAPI::Vulkan)
		{
			auto ptr = std::make_unique<impl::VulkanUniformBuffer>();
			ptr->Create(renderer, shader, (type == Shader::UniformType::Object) ? 0 : 1);
			return ptr;
		}
		return nullptr;
	}
}//namespace