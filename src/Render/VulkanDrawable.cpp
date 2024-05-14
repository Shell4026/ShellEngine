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
		indexBuffer(renderer.GetDevice(), renderer.GetGPU()),
		cmd(renderer.GetDevice(), renderer.GetCommandPool()),
		buffers(vertexBuffers),
		mat(nullptr), mesh(nullptr)
	{
		auto frameBuffer = static_cast<const impl::VulkanFramebuffer*>(renderer.GetMainFramebuffer());
		pipeline = std::make_unique<impl::VulkanPipeline>(renderer.GetDevice(), frameBuffer->GetRenderPass());
	}

	VulkanDrawable::~VulkanDrawable()
	{
		
	}

	auto VulkanDrawable::GetPipeline() const -> impl::VulkanPipeline*
	{
		return pipeline.get();
	}

	void VulkanDrawable::CreateVertexBuffer()
	{
		VkVertexInputBindingDescription bindingDesc{};
		bindingDesc.binding = 0;
		bindingDesc.stride = sizeof(glm::vec3);
		bindingDesc.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription attrDesc{};
		attrDesc.binding = 0;
		attrDesc.location = 0;
		attrDesc.format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
		attrDesc.offset = 0;

		pipeline->
			AddBindingDescription(bindingDesc).
			AddAttributeDescription(attrDesc);

		size_t size = sizeof(glm::vec3) * mesh->GetVertexCount();
		impl::VulkanBuffer stagingBuffer1{ renderer.GetDevice(), renderer.GetGPU() };
		stagingBuffer1.Create(size, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer1.SetData(mesh->GetVertex().data());

		vertexBuffers[0].Create(size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		size_t sizeIndices = sizeof(uint32_t) * mesh->GetIndices().size();
		impl::VulkanBuffer stagingBuffer2{ renderer.GetDevice(), renderer.GetGPU() };
		stagingBuffer2.Create(sizeIndices, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer2.SetData(mesh->GetIndices().data());

		indexBuffer.Create(sizeIndices,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
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

			VkBufferCopy cpyIndices{};
			cpyIndices.srcOffset = 0; // Optional
			cpyIndices.dstOffset = 0; // Optional
			cpyIndices.size = sizeIndices;
			vkCmdCopyBuffer(cmd.GetCommandBuffer(), stagingBuffer1.GetBuffer(), vertexBuffers[0].GetBuffer(), 1, &cpy);
			vkCmdCopyBuffer(cmd.GetCommandBuffer(), stagingBuffer2.GetBuffer(), indexBuffer.GetBuffer(), 1, &cpyIndices);
			}, &info);
	}

	void VulkanDrawable::Build(Material* mat, Mesh* mesh)
	{
		this->mat = mat;
		this->mesh = mesh;

		Shader* shader = mat->GetShader();
		if (shader == nullptr)
			return;

		pipeline->Clean();

		cmd.Create();
		vertexBuffers.clear();
		indexBuffer.Clean();

		vertexBuffers.push_back(impl::VulkanBuffer{ renderer.GetDevice(), renderer.GetGPU() });
		CreateVertexBuffer();
		cmd.Clean();

		cmd.Create();

		int idx = 1;
		for (auto& attr : mesh->attributes)
		{
			auto shaderAttr = shader->GetAttribute(attr.first);
			if (!shaderAttr)
				continue;

			uint32_t stride = 0;
			VkFormat format = VkFormat::VK_FORMAT_R32_SINT;
			size_t size = 0;
			const void* data;
			switch (shaderAttr->type)
			{
			case sh::render::Shader::PropertyType::Int:
				stride = sizeof(int);
				format = VkFormat::VK_FORMAT_R32_SINT;
				break;
			case sh::render::Shader::PropertyType::Float:
				stride = sizeof(float);
				format = VkFormat::VK_FORMAT_R32_SFLOAT;
				break;
			case sh::render::Shader::PropertyType::Vec2:
			{
				stride = sizeof(glm::vec4);
				format = VkFormat::VK_FORMAT_R32G32_SFLOAT;

				auto& vec = attr.second;
				size = sizeof(glm::vec4) * vec.size();
				data = vec.data();
				break;
			}
			case sh::render::Shader::PropertyType::Vec3:
			{
				stride = sizeof(glm::vec4);
				format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;

				auto& vec = attr.second;
				size = sizeof(glm::vec4) * vec.size();
				data = vec.data();
				break;
			}
			case sh::render::Shader::PropertyType::Vec4:
			{
				stride = sizeof(glm::vec4);
				format = VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;

				auto& vec = attr.second;
				size = sizeof(glm::vec4) * vec.size();
				data = vec.data();
				break;
			}
			}

			VkVertexInputBindingDescription bindingDesc{};
			bindingDesc.binding = idx;
			bindingDesc.stride = sizeof(glm::vec4);
			bindingDesc.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;

			VkVertexInputAttributeDescription attrDesc{};
			attrDesc.binding = idx;
			attrDesc.location = shaderAttr->idx;
			attrDesc.format = format;
			attrDesc.offset = 0;

			pipeline->
				AddBindingDescription(bindingDesc).
				AddAttributeDescription(attrDesc);
			
			impl::VulkanBuffer stagingBuffer{ renderer.GetDevice(), renderer.GetGPU() };
			stagingBuffer.Create(size, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_SHARING_MODE_EXCLUSIVE,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			stagingBuffer.SetData(data);

			vertexBuffers.push_back(impl::VulkanBuffer{renderer.GetDevice(), renderer.GetGPU()});
			vertexBuffers.back().Create(size,
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
				vkCmdCopyBuffer(cmd.GetCommandBuffer(), stagingBuffer.GetBuffer(), vertexBuffers.back().GetBuffer(), 1, &cpy);
				}, &info);

			++idx;
		}
		pipeline->
			SetShader(static_cast<VulkanShader*>(shader)).
			AddShaderStage(impl::VulkanPipeline::ShaderStage::Vertex).
			AddShaderStage(impl::VulkanPipeline::ShaderStage::Fragment).
			Build();

		cmd.Clean();
	}

	auto VulkanDrawable::GetVertexBuffer() const -> const impl::VulkanBuffer&
	{
		return vertexBuffers[0];
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