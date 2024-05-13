#include "VulkanDrawable.h"

#include "VulkanRenderer.h"
#include "Material.h"
#include "Mesh.h"
#include "VulkanShader.h"
#include "VulkanImpl/VulkanFramebuffer.h"

#include <cstring>

namespace sh::render
{
	VulkanDrawable::VulkanDrawable(const VulkanRenderer& renderer) :
		renderer(renderer), 
		vertexBuffer(renderer.GetDevice(), renderer.GetGPU()),
		indexBuffer(renderer.GetDevice(), renderer.GetGPU()),
		cmd(renderer.GetDevice(), renderer.GetCommandPool())
	{
		auto frameBuffer = static_cast<const impl::VulkanFramebuffer*>(renderer.GetMainFramebuffer());
		pipeline = std::make_unique<impl::VulkanPipeline>(renderer.GetDevice(), frameBuffer->GetRenderPass());

		cmd.Create();
	}

	VulkanDrawable::~VulkanDrawable()
	{
		
	}

	auto VulkanDrawable::GetPipeline() const -> impl::VulkanPipeline*
	{
		return pipeline.get();
	}

	void VulkanDrawable::Build(Material* mat, Mesh* mesh)
	{
		this->mat = mat;
		this->mesh = mesh;
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

		size_t size = sizeof(glm::vec3) * mesh->GetVertexCount();

		impl::VulkanBuffer stagingBuffer{ renderer.GetDevice(), renderer.GetGPU() };
		stagingBuffer.Create(size, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_SHARING_MODE_EXCLUSIVE, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer.SetData(mesh->GetVertex().data());

		vertexBuffer.Create(size, 
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
			VK_SHARING_MODE_EXCLUSIVE, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkCommandBufferBeginInfo info{};
		info.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		cmd.Submit(renderer.GetGraphicsQueue(), [&]() {
			VkBufferCopy cpy{};
			cpy.srcOffset = 0; // Optional
			cpy.dstOffset = 0; // Optional
			cpy.size = size;
			vkCmdCopyBuffer(cmd.GetCommandBuffer(), stagingBuffer.GetBuffer(), vertexBuffer.GetBuffer(), 1, &cpy);
		}, &info);

		cmd.Reset();
		stagingBuffer.Clean();

		stagingBuffer.Create(size, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer.SetData(mesh->GetIndices().data());

		size = sizeof(uint32_t) * mesh->GetIndices().size();
		indexBuffer.Create(size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		cmd.Submit(renderer.GetGraphicsQueue(), [&]() {
			VkBufferCopy cpy{};
			cpy.srcOffset = 0; // Optional
			cpy.dstOffset = 0; // Optional
			cpy.size = size;
			vkCmdCopyBuffer(cmd.GetCommandBuffer(), stagingBuffer.GetBuffer(), indexBuffer.GetBuffer(), 1, &cpy);
		}, &info);

		cmd.Clean();
	}

	auto VulkanDrawable::GetVertexBuffer() const -> const impl::VulkanBuffer&
	{
		return vertexBuffer;
	}

	auto VulkanDrawable::GetIndexBuffer() const -> const impl::VulkanBuffer&
	{
		return indexBuffer;
	}

	void VulkanDrawable::Update()
	{
		//vertexBuffer.SetData(mesh->GetVertex().data());
	}
}