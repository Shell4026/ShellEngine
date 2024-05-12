#include "VulkanDrawable.h"

#include "VulkanRenderer.h"
#include "Material.h"
#include "Mesh.h"
#include "VulkanShader.h"
#include "VulkanImpl/VulkanFramebuffer.h"

namespace sh::render
{
	VulkanDrawable::VulkanDrawable(const VulkanRenderer& renderer) :
		renderer(renderer)
	{
		auto frameBuffer = static_cast<const impl::VulkanFramebuffer*>(renderer.GetMainFramebuffer());
		pipeline = std::make_unique<impl::VulkanPipeline>(renderer.GetDevice(), frameBuffer->GetRenderPass());
	}

	VulkanDrawable::~VulkanDrawable()
	{
		if (vertexBuffer != nullptr)
		{
			vkDestroyBuffer(renderer.GetDevice(), vertexBuffer, nullptr);
			vertexBuffer = nullptr;
		}

		vkFreeMemory(renderer.GetDevice(), vertexBufferMem, nullptr);
	}

	auto VulkanDrawable::GetPipeline() const -> impl::VulkanPipeline*
	{
		return pipeline.get();
	}

	void VulkanDrawable::Build(Material* mat, Mesh* mesh)
	{
		VkVertexInputBindingDescription bindingDesc{};
		bindingDesc.binding = 0;
		bindingDesc.stride = sizeof(glm::vec3);
		bindingDesc.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription attrDesc{};
		attrDesc.binding = 0;
		attrDesc.location = 0;
		attrDesc.format = VkFormat::VK_FORMAT_R32G32_SFLOAT;
		attrDesc.offset = 0;

		auto shader = mat->GetShader();
		pipeline->
			SetShader(static_cast<VulkanShader*>(shader)).
			AddShaderStage(impl::VulkanPipeline::ShaderStage::Vertex).
			AddShaderStage(impl::VulkanPipeline::ShaderStage::Fragment).
			AddBindingDescription(bindingDesc).
			AddAttributeDescription(attrDesc).
			Build();

		VkDevice device = renderer.GetDevice();

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(glm::vec3) * mesh->GetVertexCount();
		bufferInfo.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create vertex buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, vertexBuffer, &memRequirements);

		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(renderer.GetGPU(), &memProperties);

		auto flag = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		uint32_t idx = 0;
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((memRequirements.memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & flag) == flag) {
				idx = i;
				break;
			}
		}

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = idx;

		if (vkAllocateMemory(device, &allocInfo, nullptr, &vertexBufferMem) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}

		vkBindBufferMemory(device, vertexBuffer, vertexBufferMem, 0);

		void* data;
		vkMapMemory(device, vertexBufferMem, 0, bufferInfo.size, 0, &data);
		memcpy(data, mesh->GetVertex().data(), (size_t)bufferInfo.size);
		vkUnmapMemory(device, vertexBufferMem);
	}

	auto VulkanDrawable::GetVertexBuffer() const -> VkBuffer
	{
		return vertexBuffer;
	}
}